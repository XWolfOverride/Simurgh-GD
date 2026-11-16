// XExt: Extension general de FenXEdit

// Grafico persecutor del raton
Process MouseSeek(graph,ax,ay);
Private mx,my;
Begin
z=-512;
Loop
 x=mx;
 y=my;
 mx=mouse.x+ax;
 my=mouse.y-ay;
 Frame;               
End;
End; 

// Interpolacion en valores fijos
Process NearValue(a,b,c);
Begin
If (a>b) a-=c;
 If (a<b) a=b;End
End
If (a<b) a+=c;
 If (a>b) a=b;End
End
Return(A);
End;

// Creditecitos..............................
Process AboutBox();
Private String title,Int OKB;
Begin
x=(640-302)/2;IsWindow=1;
y=110;
an=302;
al=227;
Dc=New_dc(an,al);
Addimage(dc,1,1,3);
AddLabel(dc,270,5,appversion);
AddLabel(dc,5,153,"Entorno de desarroyo para Fnix");
AddLabel(dc,65,130,"Fnix (C) Team Fenix");
AddLabel(dc,102,140,"(C) Jos Luis Cebrian");
OKB=AddButton(dc,218,200,70,20,"Ok");
AddImage(dc,255,15,301);
Addlabel(dc,246,45,"SeMiTeX");
AddImage(dc,255,55,300);
AddLabel(dc,251,90,"XWolf''");
AddImage(dc,255,100,302);
AddLabel(dc,242,135,"EclipseOK");
title="SeMiTeX "+appname+" "+appversion;
graph=new_window(an+13,al+28,title,0,dc);UpIt(id);
if (useAnims);
 For(alpha=0;alpha<255;alpha+=8);Frame;End;alpha=255;
end;
Loop
If (msg==MSG_REDRAW)
 unload_map(0,graph);
 graph=new_window(an,al,title,0,dc);
End;
If (msg==MSG_CLICK) Break;End;
WinProc();
Frame;
End;
msg=MSG_CLOSING;frame;
For(alpha=255;alpha>0 and useAnims;alpha-=8);Frame;End;
unload_map(0,graph);
unload_map(0,dc);
End;

// SiNoBox
Process SiNoBox(String titl,text);
Private Si,No;
Begin
signal(father,s_freeze);IsWindow=1;
x=160;
y=170;
dc=New_dc(300,61);
AddImage(dc,5,5,550);
AddLabel(dc,25,16,text);
Si=AddButton(dc,148,40,75,20,"Si");
AddButton(dc,224,40,75,20,"No");
graph=new_window(313,89,titl,0,dc);Hitag=0;UpIt(Id);
if (useAnims)
 For (alpha=0;alpha<255;alpha+=16);Frame;End;alpha=255;
end;
Loop
If (msg==MSG_REDRAW)
 unload_map(0,graph);
 graph=new_Window(313,89,titl,0,dc);
End;
If (msg==MSG_CLICK)
 If (tag==si) Hitag=1;Else;Hitag=0;End
 Break;
End;
Msg=MSG_NONE;
Winproc();
Frame;
End;
signal(father,s_wakeup);msg=MSG_CLOSING;tag=Hitag;frame;
For (alpha=255;alpha>0 and useanims;alpha-=16);Frame;End;
unload_map(0,graph);
unload_map(0,dc);
// signal(father,s_wakeup);
// frame;
End;

// MessageBox
Process MessageBox(String titl,text);
Begin
signal(father,s_freeze);IsWindow=1;
x=160;
y=170;
dc=New_dc(300,61);
AddImage(dc,5,5,551);
AddLabel(dc,25,16,text);
AddButton(dc,224,40,75,20,"Aceptar");
graph=new_window(313,89,titl,0,dc);
UpIt(id);
if (useANims)
 For (alpha=0;alpha<255;alpha+=16);Frame;End;alpha=255;
end;
Loop
If (msg==MSG_REDRAW)
 unload_map(0,graph);
 graph=new_Window(313,89,titl,0,dc);
End;
If (msg==MSG_CLICK)
 Break;
End;
Winproc();
Frame;
End;
msg=MSG_CLOSING;frame;
signal(father,s_wakeup);
For (alpha=255;alpha>0 and useanims;alpha-=16);Frame;End;
unload_map(0,graph);
unload_map(0,dc);
// signal(father,s_wakeup);
End;

process InputBox(String Titl,Text,TXT);
private tbx;
begin
signal(father,s_freeze);
isWindow=1;
x=160;
y=170;
dc=new_dc(300,61);
AddLabel(dc,5,6,text);
tbx=addText(dc,5,22,290);an=313;al=89;
tbx.hint=TXT;
control=tbx;
AddButton(dc,224,40,75,20,"Aceptar");
Graph=New_window(313,89,titl,0,dc);
UpIt(id);
if (useanims)
 for(alpha=0;alpha<255;alpha+=16);Frame;End;alpha=255;
end;
loop
if (msg==MSG_REDRAW)
 unload_map(0,graph);
 graph=new_window(313,89,titl,0,dc);
end;
if (msg==MSG_CLICK)
 if (tag==1) tbx.hint=text;end;
 break;
end;
winproc();
frame;
end;
hint=Tbx.hint;msg=MSG_CLOSING;frame;
// signal(father,s_wakeup);
for(alpha=255;alpha>0 and useanims;alpha-=16);frame;end;
unload_map(0,dc);
unload_map(0,graph);
Signal(father,s_wakeup);
UpIt(father);
Frame;
end

// Paneles
Process Panel(t);
Private show=0,ax,v,mx,my;
        String str;
Begin
graph=500;IsWindow=true;al=75;
If (t)
 x=638;
 flags=1;an=-300;
Else
 x=-289;// 1
 graph=map_clone(0,graph);
 map_put(0,graph,100,35,37);
 Map_put(0,graph,108,70,37);
 Show=1;an=300;
End;
y=404;z=-510;
Loop
If (show)
 If (t)
  ax=638;
 Else
  ax=1;
 End;
 If (collision(Type mouse))
  If (mouse.x<(x+50)AND !T) Info="Explorador "+appname;
  Else
   If (mouse.x<(x+95)AND !T) Info="Calculadora";End;
  End;
  if (!T)
   if (mouse.x>(x+289))
    if (mouse.left)
     while (mouse.left) frame;end;
     if (collision (type mouse))
      show=0;
     end
    end;
   end;
  else
   if (mouse.left)
    while (mouse.left) frame;end;
    if (collision (type mouse))
     show=0;
    end;
   end;
  end;
  If (mousedblClick)
   If (!t)
    If (mouse.x<x+50) //35+15
     DirWin(cd());
    Else
     If (Mouse.x<x+95)
      CalcBox();
     End;
    End;
   End;
  End;
 End; 
Else
 If (t)
  If (collision (Type mouse)) info="Portapapeles";End;
  ax=928;
 Else
  If (collision (Type mouse)) info="Men principal";End;
  ax=-289;
 End;
 If (mouse.left AND collision(Type mouse))
  While (mouse.left) Frame;End;
  If (collision(Type mouse))show=1;End;
 End;
End;
if (useanims)
 If (x<>ax) 
  x=NearValue(x,ax,V);
  v++;
 Else
  V=1;
 End;
else
 x=ax;
end;
Frame;
End;
End;

// InfoMANNNNNNNN
Process InfoMan();
Private String ainfo;mp;
Begin
z=-512;
Loop
 If (ainfo<>info)
  ainfo=info;
  If (Info=="")
   For (alpha=255;alpha>0;alpha-=24);
    x=mouse.x+9; If (x+an>640) x=640-an;End;
    y=mouse.y+16;If (y+14>480) y=mouse.y-14;End;
    Frame;
   End;alpha=0;
   If (graph<>0)
    unload_map(0,graph);
   End;
   graph=0;
  Else
   If (graph<>0)
    unload_map(0,graph);
   End;
   Set_text_color(cText);
   mp=write_in_map(0,Info,0);
   an=graphic_info(0,mp,g_Width)+6;
   graph=New_dc(an,14);
   map_put(0,graph,mp,3,3);
   drawing_color(cBord);
   drawing_map(0,graph);
   draw_rect(0,0,an,14);
   unload_map(0,mp);
   For (alpha=0;alpha<255;alpha+=24);
    x=mouse.x+9; If (x+an>640) x=640-an;End;
    y=mouse.y+16;If (y+14>480) y=mouse.y-14;End;
    Frame;
   End;alpha=255;
   timer[7]=0;
  End;
  // ainfo=info;
 End;
 If (timer[7]>50)
  Info="";
 End;
 x=mouse.x+9; If (x+an>640) x=640-an;End
 y=mouse.y+16;If (y+14>480) y=mouse.y-14;End
 Frame;
End;
End;

// Menu 
Process Menu(x,y,String InStr);
Private I,S;
        String Str;
        Int Text[100];
        C;
        amsg;
Begin
signal(father,s_freeze);
S=0;an=0;al=0;
Set_text_color(CText);
For (i=0;i<=len(instr);i++);
 C=asc(Instr[i]);
 If (c==10 OR I==len(Instr))
  If (str=="-")
   Text[S]=0;
  Else;
   If (str=="") str=" "; End; Text[S]=write_in_map(0,STR,0);
   If (graphic_info(0,Text[s],g_Width)>an) an=graphic_info(0,text[s],g_Width);End
  End; 
  str="";
  s++;
 Else
  If (c<>13)
   Str=str+chr(c);
  End;
 End;  
End;
al=(s*15);
s--;
an+=10;
z=-511;
graph=new_map(an,al,16);
set_center(0,graph,0,0);
map_clear(0,graph,Cbock);
drawing_map(0,graph);
drawing_color(cBord);
draw_rect(0,0,an,al);
drawing_color(cLook);
For (i=0;i<=s;i++);
 If (Text[i]==0)
  draw_line(5,4+(i*15),an-5,4+(i*15));
  draw_line(5,10+(i*15),an-5,10+(i*15));
 Else
  map_put(0,graph,Text[i],5,4+(i*15));
 End;
End;
If (x+an>640) x-=an;End;
If (y+al>480) y-=al;End;
For (alpha=0;alpha<255;alpha+=24);Frame;End;alpha=255;
Loop
 If (Incuadre(mouse.x,mouse.y,x,y,an,al))
  msg=(mouse.y-y) / 15;
 Else
  msg=-1;
 End;
 If (amsg<>msg)
  drawing_color(cBock);
  draw_rect(2,2+(amsg*15),an-2,13+(amsg*15));
  drawing_color(cBord);
  draw_rect(2,2+(msg*15),an-2,13+(msg*15));
  amsg=msg;
 End;
 If (mouse.left OR mouse.right OR mouse.middle)
  while (Mouse.left or mouse.right or mouse.middle)
   frame;
  end;
  if (!collision(type mouse)) msg=-1;end;
  Break;
 End;
 Frame;
End;
Tag=msg;
signal(father,s_wakeup);
For (alpha=255;alpha>0;alpha-=24);Frame;End;
unload_map(0,graph);
For (i=0;i<=s;i++);
 if (text[i]>0) unload_map(0,text[i]); end;
End;
// frame; // Un farme de vida de proceso para retorno de variables
End;

Process Xtring(String instr,Int num,String Pointer Outstr);
Private i,s,c;
        String str;
Begin
If (num<0) 
 [Outstr]="";
 Return(0);
End;
For (i=0;i<=len(instr);i++);
 C=asc(Instr[i]);
 If (c==13 OR I==len(Instr))
  If (s==num)
   [OutStr]=Str;
  End;
  str="";
  s++;
 Else
  If (c<>10)
   Str=str+chr(c);
  End;
 End;  
End;
End;

// El maximo
process Max(a,b);
begin
If (a>b)
 Return(a);
else
 Return(b);
end;
end;

// El minimo
Process Min(a,b);
begin
If (a<b)
 Return(a);
else
 Return(b);
end;
end;

Process CalcBox();
Private Text;
        BC,bd,bm,br,bt[10],bs,bi,bp;
        Float Num;
        Todo;
begin
IsWindow=1;
an=112;
al=168;
dc=New_dc(an,al);
Control=Text=AddText(dc,1,4,110);
bc=addbutton(dc,01,20,20,20,"C");
bd=addbutton(dc,31,20,20,20,"/");
bm=addbutton(dc,61,20,20,20,"*");
br=addbutton(dc,91,20,20,20,"-");
for (x=0;x<9;x++)
 bt[x]=addbutton(dc,01+((x mod 3)*30),50+((x / 3)*30),20,20,x+1);
 bt[x].hitag=x+1;
 bt[x].PseudoID=2;
end;
bs=addbutton(dc,91,50,20,50,"+");
bt[9]=addbutton(dc,01,140,80,20,"0");
bt[9].hitag=0;
bt[9].PseudoID=2;
bi=addbutton(dc,91,110,20,20,"=");
bp=addbutton(dc,91,140,20,20,".");
graph=New_Window(an+13,al+21,"Calculadora",0,dc);
x=rand(10,100);
y=rand(40,150);
Text.hint="0";
UpIt(id);
Icon=108;
Hint="Calculadora";
if (useanims)
 For (alpha=0;alpha<255;alpha+=16);frame;end;alpha=255;
end;
loop
 If (msg==MSG_REDRAW)
  Unload_map(0,graph);
  graph=New_Window(an,al,"Calculadora",0,dc);
 end;
 If (msg==MSG_CLICK)
  switch(tag);
   case text:
    Text.hint=ItoA(AtoI(Text.hint));
   end;
   case bc:
    Text.hint="0";
    num=0;
   end;
   Case bp:
    if (find(text.hint,".")==-1) text.hint+=".";end;
   end;
   Case bd:
    num=atof(text.hint);
    text.hint="0";
    todo=4;
   end;
   Case bm:
    num=atof(text.hint);
    Text.hint="0";
    todo=3;
   end;
   Case bs:
    num=atof(text.hint);
    text.hint="0";
    Todo=2;
   end;
   case br:
    If (text.hint=="0")
     Text.hint="-";
    else
     num=atof(text.hint);
     Text.hint="0";
     Todo=1;
    end;
   end;
   Case bi:
    Msg=MSG_RELOAD;
   end;
   case 2:
    If (Todo==128) text.hint="";todo=0;end;
    if (text.hint=="0") text.hint="";end;
    text.hint+=PseudoID.hitag;
   end;
   Case 1:
    Break;
   end;
  end;
 end;
 If (Msg==MSG_RELOAD)
  Switch (todo);
   case 1:
    text.hint=ftoa(num-atof(Text.hint));
   end;
   Case 2:
    text.hint=ftoa(num+atof(text.hint));
   end;
   Case 3:
    Text.hint=ftoa(num*atof(text.hint));
   end;
   case 4:
    Text.hint=ftoa(num/atof(text.hint));
   end;
  end;
  num=0;
  todo=128;
 end;
 WinProc();
 frame;
end;
MSG=MSG_CLOSING;frame;
For (;alpha>0 and useanims;alpha-=16);frame;end;
Unload_Map(0,graph);
Unload_map(0,dc);
end;

// --- busqueda de texto
Process FindBox();
private t1,t2;seab,repb,reab;
begin
 IsWindow=1;signal(father,s_freeze);
 an=200;
 al=100;
 dc=New_dc(an,al);
 addlabel(dc,10,10,"Texto a buscar:");
 t1=addtext(dc,10,20,180);
 addlabel(dc,10,40,"Reemplazar por:");
 t2=addtext(dc,10,50,180);
 seab=addbutton(dc,3,70,55,20,"Buscar");
 repb=addbutton(dc,65,70,75,20,"Reemplazar");
 reab=addbutton(dc,120,70,75,20,"Todo");
 graph=new_window(an+13,al+21,"Buscar y reemplazar",0,dc);
 upit(id);x=220;y=190;
 for (alpha=0;alpha<255 and useanims;alpha+=16);frame;end;alpha=255;
 loop
  if (msg==MSG_CLICK)
   if (tag==1)
    break;
   end;
  end;
  if (msg==MSG_REDRAW)
   unload_map(0,graph);
   graph=New_Window(an,al,"Buscar y reemplazar",0,dc);
  end;
  WinProc();
  frame;
 end;
 msg=MSG_CLOSING;frame;Signal(father,s_wakeUp);
 for (;alpha>0 and useanims;alpha-=16);frame;end;
 unload_map(0,graph);
 Unload_map(0,dc);
end;

// --- busqueda de procesos
process ProcBox();
private opt,scro;
        String Proc[500];
begin
 IsWindow=1;
 Signal(father,s_freeze);
 an=250;
 al=300;
 dc=new_dc(an,al);
 graph=New_Window(an+13,al+21,"Ir a proceso",0,dc);
 upit(id);
 x=195;
 y=90;
 for(alpha=0;alpha<255 and useanims;alpha+=16);frame;end;alpha=255;
 msg=MSG_REDRAW;
 loop
  if (msg==MSG_CLICK)
   if (tag==1)
    opt=0;
    break;
   end;
  end;
  if (msg==MSG_REDRAW);
   
  end;
  WinProc();
  frame;
 end;
 msg=MSG_CLOSING;
 Signal(father,s_wakeUp);
 tag=opt;
 frame;
 //for (alpha=255;alpha>0 and useanims;alpha-=16);frame;end;
 unload_map(0,graph);
 unload_map(0,dc);
end;

Begin
End;
