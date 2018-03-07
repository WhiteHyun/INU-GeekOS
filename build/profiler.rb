#!/usr/bin/ruby

nsamples = 2000
sleeptime = 0
samples = Hash.new {|h,k| h[k] = 0 }
tops = Hash.new {|h,k| h[k] = 0 }
funcs = Hash.new {|h,k| h[k] = 0 }

i386_elf = Kernel::test(?e, "/opt/local/bin/i386-elf-gdb") ? "i386-elf-" : ""
begin 
nsamples.times do 
  ary = []
  IO.popen("#{i386_elf}gdb geekos/kernel.exe -ex 'set pagination 0' -ex 'thread apply all bt' -batch", "r") { |stacks|
    stacks.readlines.each { |ln|
      if ln =~ /#(?<id>\d+)\s+(0x[\d\w]+ in )?(?<func>\w\S+)\s/ then
        # puts "line: %s" % ln
        # puts "one: %s" % Regexp.last_match(1)
        # puts "thr: %s" % Regexp.last_match(3)
        ary[Regexp.last_match(:id).to_i] = Regexp.last_match(:func)
      elsif ln =~ /^Thread / then
        samples[ary.join(',')] += 1 unless ary.empty?
        tops[ary[0]] += 1 unless ary.empty?
        # puts ary.join('-')
        ary = []
      end
    }
    stacks.close
  }
  p $? unless $?.success?
  samples[ary.join(',')]+=1;
  tops[ary[0]] += 1 unless ary.empty?
  ary.each { |f| funcs[f] += 1 }
end
rescue Exception => e
end

# don't reverse, since reversing means scrolling.
samples.sort_by{ |k,v| v}.each { |k,v| 
  puts "%d %s" % [ v,k ]
}

tops.sort_by{ |k,v| v}.reverse[0..15].each { |k,v| 
  print "%d %s; " % [ v,k ]
}

funcs.sort_by{ |k,v| v}.reverse[0..15].each { |k,v| 
  print "%d %s; " % [ v,k ]
}
puts
