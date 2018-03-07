#!/usr/bin/ruby

require 'stringio'

kthreadlocks = 0

unless Kernel::test(?e, "Makefile") then
  puts "I don't think I'm in the build directory"
  if Kernel::test(?e, "../../build/Makefile") then 
    Dir.chdir("../../build")
  else
    exit 1
  end
end

vars = ""
if Kernel::test(?x, "../../../qemu/i386-softmmu/qemu-system-i386") then
  puts "using local qemu"
  vars = "QEMU_BIN=../../../qemu/i386-softmmu/qemu-system-i386"
elsif Kernel::test(?x, "../../qemu/i386-softmmu/qemu-system-i386") then
  puts "using local qemu"
  vars = "QEMU_BIN=../../qemu/i386-softmmu/qemu-system-i386"
end
elapsed_times = []

1000.times do  |iter|
  print '[%d' % iter; $stdout.flush
  start = Time.now
  pid = Kernel.fork do 
    $stderr = StringIO.new # try harder to dump it.
    $stdout = StringIO.new
    # Kernel.system('make run %s MEM=16 ARGS=\'-nographic -testInput "multimlc 500 2\\nspawner\\nb ehlo\\nexit\\n"\' 2>&1 > /dev/null' % vars)
    Kernel.system('make run %s MEM=16 ARGS=\'-nographic -testInput "multimlc 500 2\\nb ehlo\\nexit\\n"\' 2>&1 > /dev/null' % vars)
    exit $?
  end
  begin
    print '.'
    $stdout.flush
    sleep 0.5
  end while(!Process.waitpid(pid, Process::WNOHANG)) 
  print ']' % iter; $stdout.flush
  finish = Time.now
  elapsed = finish - start
  elapsed_times.push(elapsed)
  average = elapsed_times.reduce(:+) / elapsed_times.count.to_f
  std_deviation = average # temp
  # I saw a long run that terminated, and wanted to see what it was about.
  # the two standard deviations above the mean filter is probably excessively complicated.
  if(elapsed_times.count > 4) then
    deviation = elapsed_times.inject { |dev,t| dev + ((average - t) ** 2)  }
    sample_variance = deviation/(elapsed_times.count - 1).to_f
    std_deviation = Math.sqrt(sample_variance)
  end

  ret = Kernel.system('grep -q "1 is ehlo" output.log')
  if(!ret or elapsed > average + 2 * std_deviation) then
    Kernel.system('cat output.log')
    exit 1 
  end
  ret = Kernel.system('grep kthreadlock output.log')
  kthreadlocks+=1 if(ret)
  ret = Kernel.system('grep "PIT failed" output.log')
  exit 1 if(ret)
  # puts ret
end

puts "optimistic kthread lock may have saved %d instances" % kthreadlocks
