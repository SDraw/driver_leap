Dim objShell
Set objShell = CreateObject("WScript.shell")
objShell.run "cmd /c """ & Session.Property("CustomActionData") & """"
'msgbox Session.Property("CustomActionData")
Set objShell = Nothing
