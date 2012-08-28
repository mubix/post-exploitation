## csvde.exe
src: Windows server OS

### Usage

>
**Output most of the AD object for a domain into a CSV**
<pre>csvde.exe -f domaindump.csv</pre>
**Output just Marketing objects to CSV**
<pre>csvde -d "ou=marketing,dc=contoso,dc=com" -f marketingobjects.csv</pre>

## dnscmd.exe
src: Picked off any Windows server OS with DNS role installed

### Usage

>
**Enum available zones**
<pre>dnscmd /EnumZones</pre>
**Print entire zone**
<pre>dnscmd /ZonePrint domain.com</pre>
If these commands are not done on the DNS server, one can be specified between the command and option like so:
<pre>dnscmd dc1.domain.com /EnumZones</pre>

## adfind.exe
src: http://www.joeware.net/freetools/tools/adfind/index.htm

### Usage

>
**List all domains in forest**
<pre>adfind -sc domainlist</pre>
**List all the trusts for the current domain/forest**
<pre>adfind -sc trustdmp</pre>
**List domain controllers**
<pre>adfind -sc dclist</pre>
**Lists domain controllers for another domain**
<pre>adfind -b dc=trusted,dc=otheraddomain,dc=clickycompany,dc=com -sc dclist</pre>

## nltest.exe
src: Windows OS

### Usage

>
**List all domains current domain trusts**
<pre>nltest /domain_trusts</pre>
