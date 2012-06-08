import os, platform, datetime
logfile = open(datetime.datetime.now().strftime('%Y-%m-%d-%H:%M:%S') + '.log', 'w')
for item in os.listdir('/home/andrew/Dropbox/Debian/code/instances/'):
	name = os.path.join('/home/andrew/Dropbox/Debian/code/instances/', item)
	handle = os.popen('./m 1 '+name)
	log = item.ljust(20) + handle.read().strip().rjust(55)
	print log
	logfile.write(log + '\n')
logfile.close()
