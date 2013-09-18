#NoTrayIcon
#Region ;**** Directives created by AutoIt3Wrapper_GUI ****
#AutoIt3Wrapper_UseUpx=n
#AutoIt3Wrapper_Change2CUI=y
#AutoIt3Wrapper_Run_Obfuscator=y
#Obfuscator_Parameters=/cs 1 /cn 1
#EndRegion ;**** Directives created by AutoIt3Wrapper_GUI ****
#include <Array.au3>

TCPStartup()
$ip = TCPNameToIp('backup.letmeoutofyour.net')
ConsoleWrite("letmeoutofyour.net resolved to: " & $ip & @CRLF)
$realip = '178.79.164.226'
If($ip == $realip) Then
	ConsoleWrite("Correctly resolved letmeoutofyour.net, checking ports..." & @CRLF)
	For $i = 0 to 65535
		Local $sData = InetRead("http://old.letmeoutofyour.net:" & $i & "/test")
		If BinaryToString($sData) == 'w00tw00t' & @LF Then
			ConsoleWrite("Good on port: " & $i & @CRLF)
		EndIf
	Next
Else
	ConsoleWrite("Couldn't resolve letmeoutofyour.net, using direct IP..." & @CRLF)
	For $i = 0 to 65535
		Local $sData = InetRead("http://178.79.164.226:" & $i & "/test")
		If BinaryToString($sData) == 'w00tw00t' & @LF Then
			ConsoleWrite("Good on port: " & $i & @CRLF)
		EndIf
	Next
EndIf

