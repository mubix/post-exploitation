#!/usr/bin/python -tt

import sys, socket, urllib2, threading, Queue

#=================================================
# MAIN FUNCTION
#=================================================

def main():
    import optparse
    usage = "%prog [options]\n\n%prog - Tim Tomes (@LaNMaSteR53) (www.lanmaster53.com)"
    parser = optparse.OptionParser(usage=usage)
    parser.add_option('-v', help='Enable verbose mode.', dest='verbose', default=False, action='store_true')
    parser.add_option('-p', help='Port, or range of ports (1-65535).', dest='ports', type='string', action='store')
    parser.add_option('-t', help='Number of threads. (default=5)', dest='threads', type='string', action='store')
    (opts, args) = parser.parse_args()

    if not opts.ports:
        parser.error("[!] Must provide at least one port.")
    else:
        ports = opts.ports
    threads = 5
    if opts.threads: threads = int(opts.threads)
    verbose = False
    if opts.verbose: verbose = opts.verbose
    socket.setdefaulttimeout(3)

    print "[*] Checking egress for ports %s." % ports

    if '-' in opts.ports:
        ports = range(int(opts.ports.split('-')[0]), int(opts.ports.split('-')[1])+1)
    else:
        ports = [int(opts.ports)]

    q = Queue.Queue()
    print_q = Queue.Queue()

    for i in range(threads):
        t = threading.Thread(target=doWork, args=(q,print_q,verbose))
        t.daemon = True
        t.start()

    t = threading.Thread(target=doPrint, args=(print_q,))
    t.daemon = True
    t.start()

    for i in ports:
        q.put(i)
    q.join()

#=================================================
# SUPPORT FUNCTIONS
#=================================================

def doPrint(print_q):
    while True:
        output = print_q.get()
        print_q.task_done()
        print output

def doWork(q, print_q, verbose):
    while True:
        port = q.get()
        #import pdb;pdb.set_trace()
        req = urllib2.Request('http://yo.letmeoutofyour.net:%d/hi/mubix' % port)
        try:
            res = urllib2.urlopen(req)
            if 'w00tw00t' in res.read():
                if verbose: print_q.put('[+] Port %d allowed.' % port)
            else:
                print_q.put('[!] \'w00tw00t\' not returned for port %d.' % port)
        except urllib2.URLError:
            print_q.put('[-] Port %d blocked.' % port)
        q.task_done()

#=================================================
# START
#=================================================

if __name__ == "__main__": main()
