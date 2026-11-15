// Fenxedit ][ (C) Semitex by Wolf helped by EclipseOK
Program FenXEdit2;           //------------------- IDE FenXEdit ][
Import "image"
Const AppName="FenXEdit ]["; // Nombre del editor
      AppVersion="0.31";     // y su version
      MaxCwins=7;            // Maximo de codigos simultaneos
      MaxCode=20000;         // Maximo de lineas por c¢digo
// end
Global WinSel;             // Ventana seleccionada
       ToWinSel;           // Ventana que aspira a ser seleccionada (raton encima)
       CWinsel;            // Ventana de codigo Seleccionado
       Struct CWin[Maxcwins]
        String Dir,Fil;     // Directorio y nombre del archivo de c¢digo
        int Lin,Col,Sx,Sy;  // Posicion del cursor y scroll
        Saved;              // Control de guardado
       End;
       CWins;              // Numero de ventanas de codigo
       Struct Code         // Estructura de c¢digo
        String Line[MaxCode]; // C¢digo ( Actual )
        Int Count=-1;         // Total de lineas
       End; 
       TB;                 // ToolBar ( Arriva ^ )
       LT;                 // Letras  ( Proceso de C¢digo-Editor )
       cText,cBord,cBack,cBock,cLook,cSele,cbeck; // Colores t¡picos
       bkmap=1;            // Mapa de fondo
       MouseDblClick;      // Global de doble click del raton
       MouseLock;          // Global de bloqueo del raton
       String Info;        // InfoMannnnnnnnnnn!!!
       Cur;                // Cursor;
       SiMusica;           // Control de multireproduccion de audio;
       CF;                 // Fuente para el c¢digo;
       pMenu,pPorta;       // Identificadores de paneles
       AppLevel;           // Nivel de aplicacion
       String KeyBuf;      // Buffer de teclado
       UseColors=1;        // Usar colores?
       UseCodeCut=1;       // Usar Aceleracion del c¢digo?
       UseAnims=1;         // Usar animaciones
       MleftPress;         // Anterior del raton
       Drag;               // Drag&Drop Proc

Local Bug;                 // 0.84 Bug (primera local erronea)
      Dc;                  // Device Context (Como en los Windows)
      Msg,tag,hitag,lotag; // Sistema de mensajeria
                           //  msg 1: Actualizar grafico   
                           //  msg 2: Click en boton | Tag= id_boton o (1=Cerrar)
                           //  msg 3: Al seleccionar dialogo (UpIt)
                           //  msg 4: Al modificar un control de texto | Tag= id_control
                           //  msg 5: Drop del Drag
      Ofsx,ofsy;           // Inici del DC en la ventana
      An,al;               // Ancho, Alto
      Icon;                // Icono de minimizado
      IsWindow;            // Identificadores de ventanas
      String Hint;         // Hint de InfoMannnnnnn
      Control;             // Control Seleccionado
      PseudoID;            // Id falsa para los controles
      
Private atop,v,ascan,mx,my,i,proc,atowinsel,dmx,dmy,aleft;
Begin
set_title(AppName+" "+Appversion);
Full_screen=0;             // Pantalla completa
set_mode(800,600,16);      
cText=rgb(10,10,10);       // Color del texto
cBord=rgb(80,80,80);       // Color del borde
cBack=rgb(255,248,255);    // Color del Fondo
cBock=rgb(223,216,223);    // Color del FondoOscuro
cBeck=rgb(235,235,235);
cLook=rgb(151,122,104);    // Color de los enreos
cSele=rgb(0,128,0);        // Color de seleccion
BuildSyntax();             // Genera la tabla de sintaxsis
Alpha_steps=64;            // Supertransparencias MOOOOLA
load_fpg("FenXEdit2.fpg"); // Carga de gráficos
set_icon(0,2);             // Iconcete....
set_mode(640,480,16);      // actualizar resolucion
mouse.graph=998;           // Raton
//restore_type=1;            // aceleracion inexacta, se recomienda...
//dump_type=1;
MouseSeek(999,15,-15);     // Persecutor del raton
InfoMan();                 // Manajer de informacion (el tio mas grandeee)
// Prepara la consola
Say(Appname+" "+Appversion+". (C) SeMiTeX 2003 by XWolf''");
Say("");
Say("[!] Iniciado:");
cf=load_fnt("Console.fnt");// Carga de la fuente fija
TB=TopBar();               // Barra de arriba
pMenu=Panel(0);
pPorta=Panel(1);
put_screen(0,bkmap);       // Fondo
graph=new_map(500,460,16); // Grafico negro
x=140;z=501;y=20;          // Preparacion de eso
LT=TextWin();              // Proceso de letritas
set_center(0,graph,0,0);   // Mas preparacion de eso
map_clear(0,graph,rgb(10,10,90));// Mapa negro
alpha=230;                 // Semitransparencia
atop=480;timer[9]=0;       // Inicio del temporizador del protector
set_fps(50,0);             // Corre, corre....
timer[8]=500;              // depuracion del primer dblclick
cWinsel=-1;
Keyer();                   // Demonio de teclado
Drag=Dragger();            // Demonio de Drag&Drop
Frame;
// Main Loop
Loop
 // Animacion del fondo y las letras
 If (cWinSel<0)
  atop=480;
 Else
  atop=20;
 End;
 If (UseAnims)
  If (y<>atop)
   y=NearValue(y,atop,v);v++;
  Else
   v=0;
  End;
 Else
  y=atop;
 End;
 lt.y=y;
 // Protector de pantalla;
 If (msg==0)
  If (timer[9]>6000) fade(15,15,15,1);End; 
  If ((scan_code<>ascan) OR (mouse.x<>mx) OR (mouse.y<>my) OR (mouse.left<>Aleft))
   ascan=scan_code;
   If (timer[9]>6000)
    fade_on();
   End;
   timer[9]=0;
   mx=mouse.x;my=mouse.y;
  End;
  // Evento doble click
  Mousedblclick=0;
  If (mouse.left)
   If (timer[8]<40 AND (NOT aleft))
    Mousedblclick=1;timer[8]=100;
   Else
    timer[8]=0;
   End; 
  End;
  // Seleccion de Ventana aspirante
  If (NOT MouseLock)
   Proc=get_id(0);i=0;ToWinSel=0;
   While (proc>0) 
    If (proc.IsWindow AND Proc.Lotag<>512)
     If (incuadre(mouse.x,mouse.y,proc.x,proc.y,proc.an,proc.al))
      If (i>Proc.z)
       i=Proc.z;
       ToWinSel=proc;
      End; // i>proc.z
     End;  // incuadre
    End;   // proc.IsWindow AND ...
    Proc=get_id(0);
   End;  // While
  End; // Not MouseLock
  if (atowinsel<>towinsel)
   atowinsel=towinsel;
   KeyBuf="";
  end;
 End;    // Msg==0
 // Tipo de cursor
 if (mouse.graph>900 and mouse.graph<1000)
  if (drag.PseudoID)
   Mouse.graph=997;
  else
   if (Towinsel==0 and mouse.x>184 and mouse.y>y)
    mouse.graph=996;
   else
    mouse.graph=998;
   end;
  end;
 end;
 // teketeke
 MLeftPress=(mouse.left and !aleft);
 aleft=mouse.left;
 Frame;
End;
End;

// demonio Drag&drop
Process Dragger();
 // MSG del dragger igual al tipo de drag
 //  2:Mapa
private ax,ay,oked;
begin
 Dc=0;
 loop
  if (Mouse.left and !MouseLock and PseudoID)
   Dc=1;
   ax=mouse.x;ay=mouse.y;
   oked=0;
   while (mouse.left and !oked and !MouseLock)
    oked=FGet_Dist(mouse.x,mouse.y,ax,ay)>5;
    frame;
   end;
   if (oked)
    Mouse.graph=Icon;
    Mouselock=1;
    While (mouse.left);
     frame;
    end;
    Mouse.graph=998;
    Mouselock=0;
    frame;
    If (exists(towinsel))
     toWinSel.msg=5;
    end;
   end;
   Dc=0;
  end;
  PseudoID=0;
  frame;
 end;
end;

// demonio de teclado
Process Keyer();
private aascii,i;
begin
loop
 if (key(_del))  ascii=07;end;
 if (key(_up))   ascii=14;end;
 if (key(_Down)) ascii=15;end;
 if (key(_Right))ascii=16;end;
 if (key(_Left)) ascii=17;end;
 if (key(_Home)) ascii=18;end;
 if (key(_end))  ascii=19;end;
 if (key(_pgup)) ascii=20;end;
 if (key(_pgdn)) ascii=21;end;
 if (ascii>0)
  if (aascii<>ascii or y<0)
   KeyBuf+=chr(ascii);
   If (ascii==aascii)
    y=2;
   else
    y=20;
   end;
  end;
 end;
 aascii=ascii;y--;
 frame;
end;
end;

Process NextKey()
private i;
begin
 if (len(Keybuf)==0)
  Return(0);
 end;
 i=asc(KeyBuf[0]);
 if (Len(keybuf)==1)
  Keybuf="";
 else
  Keybuf=substr(Keybuf,1,-1);
 end;
 Return(i);
end;

// Barra de arribita
Process TopBar();
Private acwins,tmp,a,boxret,scwin,acwinsel;
        MenuStr="Cerrar
-
Guardar
Guardar como...
-
Recargar (Descartar cambios)";
        HelpMenu="Ayuda de Fenxedit ][
Ayuda de Fenix
-
Acerca de FenXEdit ][";
Begin
// Crea el grafico
graph=new_map(640,20,16);
set_center(0,graph,0,0);
x=0;y=0;acwins=-1;
Loop
// Si se ha modificado, se repinta
If (acwins<>cwins or acwinsel<>cwinsel or Msg==1)
 msg=0;
 acwins=cwins;
 acwinsel=cwinsel;
 map_clear(0,graph,0);
 drawing_map(0,graph);
 drawing_color(cText);
 draw_rect(1,1,639,19);
 drawing_color(cBack);
 draw_box(2,2,638,18);
 // Botones Cerrar, Acerca de
 map_put(0,graph,510,3,3);
 map_put(0,graph,511,18,3);
 // Botones dabajo
 If (cwins>0)
  an=(605 / cwins);
  For (a=0;a<cwins;a++)
   Tmp=New_Button(an-1,14,cWin[a].Fil,a);
    map_put(0,graph,tmp,33+(a*an),3);
    if (!cwin[a].saved) map_put(0,graph,600,40+(a*an),10);end;
   unload_map(0,TMP);
  End;
 End;
End;
// On Click;
If (collision(Type mouse))
 If (mouse.x<17)
  Info="Salir de FenXEdit ][";
 Else
  If (mouse.x<32)
   Info="Acerca de FenXEdit ][";
  Else
   If (cwins>0)
    scwin=(mouse.x-33)/(605 / cwins);
   Else
    scwin=-2;
   End;
  End; 
 End;
 If (mouse.left)
  While(mouse.left) Frame;End;
  if (Collision(type mouse))
   // On Exit
   If (mouse.x<17)
    winsel=SiNoBox("Ein? :(","¨Seguro que desea salir de FenXEdit ][??");Frame;
    If (winsel.hitag)
     if (cWinSel>=0) CloseCode();end;
     father.msg=1;
     fade(0,0,0,2);While (fading);Frame(3);End;
     exit();
    End;
   Else
    // On About
    If (mouse.x<32)
     LoTag=Menu(17,20,HelpMenu);Frame;
     Switch (Lotag.tag);
      Case 0:
       XHelpBox();
      End;
      Case 1:
       HelpBox();
      end;
      Case 3:
       AboutBox();
      End; 
     End;
    Else
     if (cWins>0)
      if (scwin==cWinsel)
       CloseCode();
       cWinsel=-1;
      else
       CloseCode();
       cwinsel=scwin;
       OpenCode();
       lt.msg=1;
      end;
     end;
    End; 
   End;  
  End; 
 End; 
 If (mouse.right and scwin==cWinsel)
  If (mouse.x>32 & cWins>0)
   lotag=Menu(mouse.x,mouse.y,Menustr);Frame;
   switch (lotag.tag)
    case 0:
     CloseCode();
     for (tmp=scwin;tmp<cwins;tmp++)
      cWin[tmp].dir=cWin[tmp+1].dir;
      cWin[tmp].fil=cWin[tmp+1].fil;
      cWin[tmp].saved=cWin[tmp].saved;
      cWin[tmp].lin=cWin[tmp].lin;
      cWin[tmp].col=cWin[tmp].col;
      cWin[tmp].sx=cWin[tmp].sx;
      cWin[tmp].sy=cWin[tmp].sy;
     end;
     cWinsel=-1;
     cWins--;
    end;
    Case 2:
     CloseCode();
    end;
    Case 5:
     OpenCode();
    end;
   end;
  End;
 End;
End;
Frame;
End;
End

Include "XDir.prg";
Include "XExt.prg";
Include "XControl.prg";
Include "XSound.prg";
Include "XSyntax.prg";
Include "XCode.prg";
Include "XMap.prg";
Include "XFpg.prg";
Include "XFnt.prg";
Include "XHelp.prg";
