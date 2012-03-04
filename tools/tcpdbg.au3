#include <ButtonConstants.au3>
#include <EditConstants.au3>
#include <GUIConstantsEx.au3>
#include <GuiIPAddress.au3>
#include <StaticConstants.au3>
#include <WindowsConstants.au3>
Opt("GUIOnEventMode", 1)


$Form1 = GUICreate("TCP Debug", 721, 641, 254, 143)
GUISetOnEvent($GUI_EVENT_CLOSE,    "Form1_Close")
GUISetOnEvent($GUI_EVENT_MINIMIZE, "Form1_Minimize")
GUISetOnEvent($GUI_EVENT_MAXIMIZE, "Form1_Maximize")
GUISetOnEvent($GUI_EVENT_RESTORE,  "Form1_Restore")

$Group1 = GUICtrlCreateGroup(" Send Data ", 8, 42, 704, 288)

GUICtrlCreateLabel("Dst IP ", 24, 14, 50, 25, $SS_CENTERIMAGE)
$IPAddr = _GUICtrlIpAddress_Create($Form1, 84, 16, 164, 20)
_GUICtrlIpAddress_Set($IPAddr, "127.0.0.1")
GUICtrlCreateLabel(":", 258, 14, 50, 25, $SS_CENTERIMAGE)
$I_Port = GUICtrlCreateInput("8080", 274, 16, 65, 20)
$C_Keepalive = GUICtrlCreateCheckbox("Keep alive", 356, 14, 80, 25)
    GUICtrlSetOnEvent(-1, "C_Keepalive_Click")
$C_AutoConn = GUICtrlCreateCheckbox("Auto connect", 528, 14, 96, 25)
    GUICtrlSetOnEvent(-1, "C_AutoConn_Click")
$B_Conn = GUICtrlCreateButton("Connect", 632, 14, 64, 25)
    GUICtrlSetOnEvent(-1, "C_Conn_Click")
$Edit1 = GUICtrlCreateEdit("", 23, 64, 596, 252)
    GUICtrlSetData(-1, "GET /test.ext?Aa=00&Bb=11&Cc=22" & @CRLF & @CRLF)
    GUICtrlSetOnEvent(-1, "Edit1_Change")
$B_Send = GUICtrlCreateButton("Send", 632, 72, 65, 25)
    GUICtrlSetOnEvent(-1, "C_Send_Click")
    GUICtrlSetState(-1, $GUI_DISABLE)
$C_Clear_S = GUICtrlCreateButton("Clear", 632, 112, 65, 25)
    GUICtrlSetOnEvent(-1, "C_Clear_S_Click")

$Group2 = GUICtrlCreateGroup(" Recv Data ", 8, 344, 704, 288)
$Edit2 = GUICtrlCreateEdit("", 23, 366, 596, 252, BitOR($GUI_SS_DEFAULT_EDIT,$ES_READONLY))
    GUICtrlSetOnEvent(-1, "Edit2_Change")
$C_Clear_R = GUICtrlCreateButton("Clear", 632, 408, 65, 25)
    GUICtrlSetOnEvent(-1, "C_Clear_R_Click")
$C_Auto_CR = GUICtrlCreateCheckbox("Auto", 640, 440, 50, 25)
    GUICtrlSetOnEvent(-1, "C_Auto_CR_Click")
    GUICtrlSetState(-1, $GUI_CHECKED)

$C_RecvByte = GUICtrlCreateLabel("1024B", 628, 600, 64, 17, $SS_RIGHT)
$C_SendByte = GUICtrlCreateLabel("1024B", 628, 295, 64, 17, $SS_RIGHT)

GUISetState(@SW_SHOW)

Dim $Socket = 0
Opt("TCPTimeout", 100)
TCPStartup()

While 1
    If $Socket > 0 Then
        $Recv = TCPRecv($Socket, 4096)
        If $Recv == "" Then
            If @error == -1 Then SocketClose()
        Else
            If GUICtrlRead($C_Auto_CR) == $GUI_CHECKED Then
                GUICtrlSetData($Edit2, $Recv)
            Else
                GUICtrlSetData($Edit2, GUICtrlRead($Edit2) & $Recv)
            EndIf
        EndIf
    EndIf
	Sleep(10)
WEnd


Func SocketConnect()
  GUICtrlSetState($B_Conn, $GUI_DISABLE)
  $IP = _GUICtrlIpAddress_Get($IPAddr)
  $Port = GUICtrlRead($I_Port)
  $Socket = TCPConnect($IP, $Port)
  If $Socket > 0 Then
    GUICtrlSetData($B_Conn, "Disconn")
    GUICtrlSetState($B_Conn, $GUI_ENABLE)
    GUICtrlSetState($B_Send, $GUI_ENABLE)
  Else
    If @error == 1 Then
      MsgBox(4096, "", "IPAddr is incorrect.")
    ElseIf @error == 2 Then
      MsgBox(4096, "", "Port is incorrect.")
    Else
      MsgBox(4096, @error, "Can not connect to " & $IP & ":" & $Port)
    EndIf
    GUICtrlSetState($B_Conn, $GUI_ENABLE)
    TCPCloseSocket($Socket)
  EndIf
EndFunc


Func SocketClose()
  TCPCloseSocket($Socket)
  GUICtrlSetData($B_Conn, "Connect")
  GUICtrlSetState($B_Send, $GUI_DISABLE)
  $Socket = 0
EndFunc


Func C_Conn_Click()
  If $Socket > 0 Then
    SocketClose()
  Else
    SocketConnect()
  EndIf
EndFunc

Func C_Auto_CR_Click()
EndFunc

Func C_Clear_R_Click()
  GUICtrlSetData($Edit2, "")
EndFunc


Func C_Clear_S_Click()
  GUICtrlSetData($Edit1, "")
EndFunc


Func C_AutoConn_Click()

EndFunc


Func C_Send_Click()
    If $sock <= 0 And  Then
    
    Endif
    $n = TCPSend($Socket, GUICtrlRead($Edit1))
    If $n == 0 Then
        SocketClose()
    EndIf
EndFunc


Func C_Keepalive()

EndFunc


Func Edit1_Change()

EndFunc


Func Edit2_Change()

EndFunc


Func Form1_Close()
  TCPShutdown()
  Exit
EndFunc


Func Form1_Maximize()

EndFunc


Func Form1_Minimize()

EndFunc


Func Form1_Restore()

EndFunc
