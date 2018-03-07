#!/usr/bin/ruby

require 'socket'
require 'digest/md5'
include Socket::Constants

qemus = ["/Applications/Q.app/Contents/MacOS/i386-softmmu.app/Contents/MacOS/i386-softmmu", "/usr/bin/qemu" ]
qemus = ["/opt/local/bin/qemu", "/usr/bin/qemu", "/usr/local/bin/qemu" ]
qemus = ["/opt/local/bin/qemu", "/usr/local/bin/qemu", "/usr/bin/qemu" ]

StdVga = true ? '' : '-std-vga'
SendARPs = false

QEMU=qemus.find { |v| Kernel::test(?x, v) } 

set_keyboard = if QEMU != "/usr/local/bin/qemu" then
                 "-k en-us"
               else
                 ""
               end

IO.popen("#{QEMU} --version").each { |ln|
  if ln =~ /QEMU PC emulator version (\d+\.\d+\.\d+)/ then
    $qemu_major, $qemu_minor, $qemu_revision = $1.split('.').map { |i| i.to_i }
  end
}

unless $qemu_major
  $qemu_major, $qemu_minor, $qemu_revision = [ 0, 0, 0 ]
end

# Wiring = [ [ "1:eth0", "2:eth0" ], [ "1:eth1", "3:eth0" ] ]
Wiring = [ [ "1:eth0", "2:eth0" ] ]
DebugInstance = nil
DEBUG_PORT = 11212 

$interface_for_accept_socket = Hash.new
accepting_ports = Hash.new
Wiring.flatten.sort.uniq.each { |interface|
  socket = Socket.new( AF_INET, SOCK_STREAM, 0 )
  sockaddr = Socket.pack_sockaddr_in( 0, 'localhost' )
  socket.bind( sockaddr )
  socket.listen( 5 )
  port, localhost = Socket.unpack_sockaddr_in(socket.getsockname)
  $interface_for_accept_socket[socket] = interface
  accepting_ports[port] = interface
}

AcceptingPorts = accepting_ports

Instances = AcceptingPorts.values.map { |t| t.split(':')[0] }.uniq

# AcceptingPorts.map { |port, interface|
  # s = TCPServer.new('127.0.0.1', port) 
  # s.listen(5)
  # $interface_for_accept_socket[s] = interface
# }

$in_from_interface_out_to = Hash.new
$interface_is_on_net = Hash.new
Wiring.each_with_index { |ary,net|
  ary.each { |iface|
    $in_from_interface_out_to[iface] = ary - [iface]
    $interface_is_on_net[iface] = net
  }
}


$interface_for_socket = Hash.new
$ip_for_interface = Hash.new

pids = []

vlan_counter = 0
Instances.each { |instance| 
  i = 0
  name = "GeekOS:"
  puts "working on instance #{instance}" if $VERBOSE
  nics = AcceptingPorts.to_a.select { |k,v| v =~ /^#{instance}:/ }.map { |port,interface|
    puts "set #{interface} to network #{$interface_is_on_net[interface]}" if $VERBOSE
    vlan_counter += 1
    $ip_for_interface[interface] = "169.254.%d.%d" % [ ($interface_is_on_net[interface] << 4), vlan_counter ]
    name += " " + $ip_for_interface[interface]
    "-net nic,model=ne2k_isa,vlan=%d,macaddr=52:54:00:12:%d0:%d -net socket,vlan=%d,connect=127.0.0.1:%d" %
    [vlan_counter, $interface_is_on_net[interface], vlan_counter, vlan_counter, port ]
  }
  pids.push Kernel.fork {
    # yes, it sucks, but acpi claims irq 9 on darwin, then the first net card conflicts and qemu drops.
    # it's a qemu problem.  sigh.
    if(DebugInstance == instance.to_i) then
      name += " DEBUG"
    end
    command = "%s -name '#{name}' %s -no-acpi %s -m 10 #{StdVga} diskc.img" % [ QEMU, set_keyboard, nics.join(' ') ]
    if(DebugInstance == instance.to_i) then
      o = File.open(".gdbinit", "w")
      o.puts "target remote localhost:#{DEBUG_PORT}"
      o.close
      if($qemu_minor >= 12) then
        command += " -S -gdb tcp::#{DEBUG_PORT}" # the mac version from ports uses this syntax.
      else
        command += " -s -S -p #{DEBUG_PORT}" # the typical linux version uses this.
      end
    end
# -net nic,model=ne2k_isa,vlan=%d,macaddr=52:54:00:12:%d0:%d -net socket,vlan=%d,connect=127.0.0.1:%d
    puts command if $VERBOSE
    unless Kernel.system(command) then
      puts "failed to run #{command}: #{$!}"
    end
  }
}

raise "no pids." if pids.empty? 
begin
  sleep 1
  puts "waiting" if $VERBOSE
  while (pid = Process.waitpid(-1, Process::WNOHANG)) do
    pids.delete(pid)
  end
rescue Errno::ECHILD
  fail "no child qemus survived" 
end
raise "no pids survived" if pids.empty? 

def mac(str) 
  (0..5).map { |i| "%02x" % str.getbyte(i) }.join(':')
end
def ip(str) 
  (0..3).map { |i| "%u" % str.getbyte(i) }.join('.')
end
def ipp(proto) 
  case proto
  when 17 then "UDP"
  when 6 then  "TCP"
  else "Proto:%d" % proto
  end
end
def hexy(str)
  (0..str.length).map { |i| "%02x" % str[i] }.join(' ')
end
def flags(tcp_flags)
  str = ""
  str += "F" if (tcp_flags & 0x1 )
  str += "S" if (tcp_flags & 0x2 )
  str += "R" if (tcp_flags & 0x4 )
  str += "P" if (tcp_flags & 0x8 )
  str += "." if (tcp_flags & 0x10)
  str += "U" if (tcp_flags & 0x20)
  return "%s(0x%x)" % [ str, tcp_flags ]
end


def dump_message(interface, contents)
  dest_mac, source_mac, ether_size, payload = contents.unpack("a6a6na*") # is there crc?
  raise "missing contents" unless ether_size
  if ether_size == 0x0800 then 
    ip_vhl, ip_tos, ip_len, ip_id, ip_frag, ip_ttl, ip_proto, ip_cksum, ip_src, ip_dst, packet = 
      payload.unpack("CCnnnCCna4a4a*")
    case ip_proto
    when 17 then # udp
      udp_sport, udp_dport, udp_length, udp_cksum, datagram = packet.unpack("nnnna*")
      puts "%s: %s->%s (udp) %s:%d->%s:%d %s" % [ interface, mac(source_mac), mac(dest_mac), ip(ip_src), udp_sport, ip(ip_dst), udp_dport,  (0..datagram.length).map { |i| "%02x" % datagram[i] }.join(' ') ]
    when 6 then # tcp
      tcp_sport, tcp_dport, tcp_seqno, tcp_ackno, tcp_offset, tcp_flags, tcp_rwnd, tcp_cksum, tcp_urg, segment =
        packet.unpack("nnNNCCnnna*")
      puts "%s: %s->%s (tcp) %s:%d->%s:%d %s %d:%d %s" % [ interface, mac(source_mac), mac(dest_mac), ip(ip_src), tcp_sport, ip(ip_dst), tcp_dport,  flags(tcp_flags), tcp_seqno, tcp_seqno+segment.length, (0..segment.length).map { |i| "%02x" % segment[i] }.join(' ') ]
    else
      puts "%s: %s->%s (ip?) %s->%s %s %s" % [ interface, mac(source_mac), mac(dest_mac), ip(ip_src), ip(ip_dst), ipp(ip_proto), (0..packet.length).map { |i| "%02x" % packet[i] }.join(' ') ]
    end
  elsif ether_size == 0x0806 then
    arp_hrd, arp_proto, arp_hlen, arp_plen, arp_op, arp_sha, arp_spa, arp_tha, arp_tpa = payload.unpack('nnCCna6a4a6a4')
    puts "%s: %s->%s (arp) %s len%d %s %s %s %s" % [ interface, mac(source_mac), mac(dest_mac), arp_op==1 ? "req" : "resp",  
      contents.length,
      mac(arp_sha), ip(arp_spa), mac(arp_tha), ip(arp_tpa) ]
  else
    puts "%s: %s->%s (%d) %s (overall size: %d)" % [ interface, mac(source_mac), mac(dest_mac), ether_size, (0..(payload.length-1)).map { |i| "%02x" % payload.getbyte(i) }.join(' '), contents.length ]
  end
end

def process_message(interface, contents) 
  dump_message(interface, contents)

  forward_to = $in_from_interface_out_to[interface]
  $interface_for_socket.each { |socket, interface|
    if forward_to.include?(interface) then
      puts "mux forwarding to #{interface}/#{socket}" if $VERBOSE
      socket.write([ contents.length, contents ].pack("Na*") )
    end
  }
end

def socket_for_interface(interface)
  socket = nil
  $interface_for_socket.each { |s,interface|
    if interface == on_interface then
      socket = s
      break
    end
  }
end

def arp_query(on_interface, for_ip, socket) 
  begin
  my_addr = "Spring"
  my_ip_on_interface = $ip_for_interface[on_interface].gsub(/\.\d+$/, '.254').split('.').map { |v| v.to_i }.pack('CCCC')
  for_ip_encoded = for_ip.split('.').map { |v| v.to_i }.pack('CCCC')
    zz =" "
    zz[0] = 0
  query_addr = zz * 6
  ff=" "
  ff[0] = 255 
  bcast_addr  = ff * 6
  arp_message = [ 1, 0x800, 6, 4, 1, my_addr, my_ip_on_interface, query_addr, for_ip_encoded ].pack('nnCCna6a4a6a4')
  ether_message = [ bcast_addr, my_addr, 0x806, arp_message ].pack('a6a6na*')

  dump_message(on_interface.gsub(/:/,'='), ether_message);

  encoded_message = [ ether_message.length, ether_message ].pack('Na*')
    # puts hexy(encoded_message)
  socket.write( encoded_message )
  rescue => e
    puts "failed to arp_query: #{e} #{e.backtrace.join('\n\t')}"
  end
end

# arp_query( $interface_for_accept_socket[process_me], $ip_for_interface[ $interface_for_accept_socket[process_me] ])

def periodic_arp_test(interface, socket) 
  if SendARPs then
    sleep 3 + socket.fileno; # to get some offset.
    while true
      arp_query(interface, $ip_for_interface[interface], socket)
      # should probably have a way of collecting the response.
      sleep 10;
    end
  end
end

begin
$storage_buffer = Hash.new { |h,k| h[k] = "" }
while true
  ready = IO.select($interface_for_accept_socket.keys + $interface_for_socket.keys , nil, nil, 60)
  if ready then
    ready[0].each { |process_me|
      if($interface_for_accept_socket.has_key?(process_me)) then
        puts "mux accepted %s" % $interface_for_accept_socket[process_me] if $VERBOSE
        new_socket, unused_client_addr = process_me.accept
        $interface_for_socket[new_socket] = $interface_for_accept_socket[process_me]
        Thread.new($interface_for_socket[new_socket], new_socket) { |interface, socket| periodic_arp_test(interface, socket) }
      else
        begin
          # frame format off qemu is a unsigned int length in net byte order, then frame.
          new_data = $storage_buffer[process_me] + process_me.readpartial(1024)
          if new_data && !new_data.empty? then
            while new_data do
              if new_data.length > 4 then
                internal_length = new_data.unpack("N")[0]
                if new_data.length >= 4 + internal_length then
                  process_message($interface_for_socket[process_me], new_data[4..4+internal_length])
                  new_data = new_data[4+internal_length..-1]
                  puts "%d characters leftover" % new_data.length unless new_data.empty?
                else
                  $storage_buffer[process_me] = new_data
                  new_data = nil
                end
              else
                $storage_buffer[process_me] = new_data
                new_data = nil
              end
            end
            # arp_query($interface_for_socket[process_me], $ip_for_interface[$interface_for_socket[process_me]], process_me)
          else # no new data read 
            puts "end of file?"
            #        $interface_for_socket[process_me].eof(process_me)
                # $storage_buffer[process_me].length unless $storage_buffer[process_me].empty?
                # $storage_buffer[process_me] 
          end
        rescue EOFError
          puts "%s went away (eof)" % $interface_for_socket[process_me]
          process_me.close
          $interface_for_socket.delete(process_me);
          # $interface_for_socket[process_me].eof(process_me)
        end
      end
    }
  else
    puts "select timeout.  active connections on #{$interface_for_socket.length} sockets"
  end
end
  
rescue => e
puts e
puts e.backtrace
pids.each { |pid|
  Process.kill("INT", pid)
}
end




