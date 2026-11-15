// XControl: Unidad de Controles

Process New_Window(an,al,String titl,Int t,Sub);
Private mp,a;
Begin
graph=new_map(an,al,16);
father.an=an;
father.al=al;
set_center(0,graph,0,0);
// Mapa interno
If (map_exists(0,Sub))
 MP=map_clone(0,sub);
 set_center(0,mp,0,0);
 map_put(0,graph,mp,6,21);
 unload_map(0,mp);
End
// Offsets del marco de la ventana
father.ofsx=6;
father.ofsy=21;
For (a=20;a<(an-20);a+=18);
 map_put(0,graph,502,a,0);
 map_put(0,graph,507,a,al-7);
End;
For (a=20;a<(al-7);a+=6);
 map_put(0,graph,504,0,a);
 map_put(0,graph,505,an-7,a);
End;
Titl=" "+titl+" ";
AddLabel2(graph,((an-text_width(0,titl)) / 2)-1,07,titl,cText,cBock,-1,8,0);
map_put(0,graph,501,0,0);
map_put(0,graph,503,an-20,0);
map_put(0,graph,506,0,al-7);
map_put(0,graph,508,an-20,al-7);
Return(graph);
End;

Process New_Button(an,al,String txt,Int index);
Private TMP,c1,c2;
Begin
graph=new_map(an,al,16);
If (cwinsel<>index)
 c1=cBock;
 c2=cBack;
Else
 c1=cText;
 c2=cBock;
End;
set_center(0,graph,0,0);
drawing_map(0,graph);
drawing_color(cBord);
draw_rect(0,0,an,al);
drawing_color(c2);
draw_box(1,1,an-1,al-1);
Set_text_color(c1);
TMP=write_in_map(0,txt,4);
 map_put(0,graph,tmp,an / 2,al / 2);
unload_map(0,tmp);
Return(graph);
End;

Process New_DC(an,al);
Begin
graph=new_map(an,al,16);
set_center(0,graph,0,0);
map_clear(0,graph,rgb(255,254,255));
Return(graph);
End;

Process AddLabel(dc,x,y,String text);
Begin
Set_text_color(rgb(10,10,10));
graph=write_in_map(0,text,0);
map_put(0,dc,graph,x,y);
unload_map(0,graph);
End;

Process AddLabel2(dc,x,y,String text,Int t,f,an,al,cm);
Private mp;ax,ay;
Begin
Set_text_color(t);
mp=write_in_map(0,text,cm);
If (an==-1)
 An=graphic_info(0,mp,G_Width);
End;
graph=new_map(an,al,16);
set_center(0,graph,0,0);
map_clear(0,graph,f);
ax=((cm MOD 3)*an)/ 2;
ay=((cm / 3)*al) / 2;
map_put(0,graph,mp,ax,ay);
map_put(0,dc,graph,x,y);
unload_map(0,mp);
unload_map(0,graph);
End;

Process AddText(dc,x,y,an);
private mp,mpx,i,color,acontrol,atowinsel;
        string ahint="*_*_*_*_*";
begin
al=12;
msg=512;
tag=-2;
mpx=New_map(an,al,16);
set_center(0,mpx,0,0);
loop
 if (hint<>Ahint or acontrol<>father.control or towinsel<>atowinsel)
  if (father.control==id and Father==towinsel and father==winsel)
   Color=cBock;
  else
   Color=cBack;
  end;
  map_clear(0,mpx,color);
  drawing_map(0,mpx);
  drawing_color(cbord);
  set_text_color(ctext);
  mp=write_in_map(0,hint,0);
  Map_put(0,mpx,mp,-tag,2);
  draw_rect(0,0,an,al);
  if (color==cBock)
   i=Text_Width(0,hint)+2;
   Draw_box(i,2,i+3,10);
  end;
  Map_put(0,dc,mpx,x,y);
  father.msg=1;
  ahint=hint;
  Unload_Map(0,mp);
  acontrol=father.control;
  atowinsel=towinsel;
 end;
 if (winsel==Father)
  if (incuadre(mouse.x,mouse.y,x+father.ofsx+father.x,y+father.y+father.ofsy,an,al) and mouse.left)
   Father.Control=id;
  end;
 end;
 if (Father.control==id and father==Towinsel)
  if ((i=NextKey())<>0)
   Father.Msg=4;
   Father.Tag=id;
   Father.Lotag=Lotag;
   Father.Hitag=Hitag;
   switch (i)
    Case 8:
     if (Len(hint)==1)
      hint="";
     else
      hint=substr(hint,0,-2);
     end;
    end;
    Case 13:
     Father.Msg=2;
     Father.Tag=id;
     Father.Lotag=Lotag;
     Father.Hitag=Hitag;
    end;
    Case 32..255:
     hint+=chr(i);
    end;
   end;
  end;
 end;
frame;
IF (FATHER.MSG==256) BREAK;END;
end;
unload_map(0,mpx);
end;

Process AddImage(dc,x,y,img);
Begin
graph=map_clone(0,img);
set_center(0,graph,0,0);
map_put(0,dc,graph,x,y);
unload_map(0,graph);
End;

Process AddImage2(dc,x,y,fpg,img,size);
Begin
 graph=map_clone(fpg,img);
 set_center(0,graph,0,0);
 map_xput(0,dc,graph,x,y,0,size,0);
 unload_map(0,graph);
End;

Process AddVolBar(dc,x,y);
Private atag,a;
Begin
tag=127;atag=127;
a=map_clone(0,50);
set_center(0,a,0,0);
map_put(0,dc,a,x,y);
Loop
If (tag<>atag)
 unload_map(0,a);
 a=new_map(63,11,16);
 map_clear(0,a,cBack);
 set_center(0,a,0,0);
 map_put(0,a,50,0,0);
 drawing_map(0,a);
 drawing_color(cBack);
 atag=((tag*63) / 127);
 draw_box(atag,0,63,11);
 map_put(0,dc,a,x,y);
 atag=tag;
 father.msg=1;Frame;
End;
If (father==winsel)
 If (mouse.left AND Incuadre(mouse.x,mouse.y,father.x+father.ofsx+x,father.y+father.ofsy+y,63,11))
  Tag=((mouse.x-(father.x+father.ofsx+x))*127) / 63;
  father.msg=2;
  father.tag=id;
  Info=itoa((TAG*100)/127)+"%";
 End;
End;
Frame;
If (father.msg==256) Break;End
End;
End;

Process AddButton(dc,x,y,an,al,String text);
Private bt,bu,bd,b,ab,aleft;
Begin
bt=MakeBTNImage(an,al,text,cBock,cText);
bu=MakeBTNImage(an,al,text,cBack,cText);
bd=MakeBTNImage(an,al,text,cText,cBock);
map_put(0,dc,bt,x,y);b=bt;ab=b;
Pseudoid=id;
Loop
If (father==winsel)
 If (incuadre(mouse.x,mouse.y,x+father.x+father.ofsx,y+father.y+father.ofsy,an,al))
  info=hint;
  If (mouse.left) 
   b=bd;
  Else
   b=bu; 
  End;
 Else
  b=bt; 
 End; 
 If (ab<>b)
  father.msg=1;
  map_put(0,dc,b,x,y);
  ab=b;
  Frame;
 end; 
 If (!mouse.left and aleft and b==bu)
  father.msg=2;
  father.hitag=hitag;
  father.lotag=lotag;
  father.tag=PseudoId;
  Father.PseudoID=id;
  Frame;
 End;
 aleft=mouse.left;
End;
If (father.msg==256) 
 Break;
end
Frame;
If (father.msg==256) Break;End
End;
unload_map(0,bt);
unload_map(0,bd);
unload_map(0,bu);
End;

Process incuadre(ax,ay,x,y,an,al);
Begin
// return(
if (an<0) x+=an;an=-an;end
If (((ax>x) AND (ax<x+an)) AND ((ay>y) AND (ay<y+al)))
 Return(1);
End;
Return(0);
// );
End;

Process ILabel(dc,x,y,String text,Int color);
Begin
Set_text_color(color);
graph=write_in_map(0,text,4);
map_put(0,dc,graph,x,y);
unload_map(0,graph);
End;

Process MakeBTNImage(an,al,String text,Int cf,ct);
Begin
graph=new_map(an,al,16);
set_center(0,graph,0,0);
drawing_map(0,graph);
drawing_color(cBord);
draw_line(1,0,an-1,0);
draw_line(0,1,0,al-1);
draw_line(1,al-1,an-1,al-1);
draw_line(an-1,1,an-1,al-1);
drawing_color(cf);
draw_box(1,1,an-1,al-1);
ILabel(graph,an/2,al/2,text,cT);
Return(graph);
End;

Process WinProc(); // to do whith a function
Private ax,ay,Todo;
        String Leko="Restaurar
Cerrar";//
Begin
father.Msg=0;
If (winsel==father)
 If (Incuadre(mouse.x,mouse.y,(father.x+father.an)-18,father.y+5,10,10) AND (father.icon<>0))
  Info="Minimizar";
  ToDo=1;
 Else 
  If (incuadre(mouse.x,mouse.y,father.x+6,father.y+5,10,10))
   Info="Cerrar";
   ToDo=2;
  End; 
 End;
 If (incuadre(mouse.x,mouse.y,father.x,father.y,father.an,21) AND MLeftPress)
   // Cerrado
   If (todo==2)
    signal(father,s_freeze);Mouselock=1;
    While (mouse.left) Frame;End;
    signal(father,s_wakeup);MouseLock=0;
    If (incuadre(mouse.x,mouse.y,father.x+6,father.y+5,10,10))
     father.msg=2;
     father.tag=1;
     Frame;
    End;
  Else
   // Iconizado
   If (Todo==1)
    signal(father,s_sleep);
    x=20;
    y=380;
    ay=type winproc;
    ax=get_id(ay);
    graph=Father.icon;
    while (ax<>0)
     if (ax<>id)
      if (collision(ax))
       x+=33;
       If (x>130)
        y-=33;
        x=20;
       end;
       frame(1);
      end;
     end; 
     ax=get_id(ay);
    end;
    z=10;
    Mouselock=1;
    While (mouse.left) Frame;End;
    MouseLock=0;father.lotag=512;hitag=0;
    While (graph<>0)
     If (collision(Type mouse))
      info=father.hint;
      If (MouseDblClick)
       graph=0;
      Else
       If (mouse.left AND (NOT Mouselock))
        alpha=200;graph=0;mouse.graph=father.icon;Mouselock=1;
         While (mouse.left)
          x=mouse.x;
          y=mouse.y;
          Frame;
         End;graph=father.icon;mouse.graph=998;
        alpha=255;Mouselock=0;
       End;
      End;
      If (mouse.right)
       tag=Menu(mouse.x,mouse.y,Leko);Frame;
       If (tag.msg==0); graph=0;End;
       If (tag.msg==1); graph=0;Hitag=1;End;
      End;
     End;
     Frame;
    End;
    signal(father,s_wakeup);father.lotag=0;upit(father);
    if (Hitag==1)
     Father.msg=2;
     Father.tag=1;
    end;
   Else
    If (NOT mouselock)
     // Movido
     ax=mouse.x-father.x;MouseLock=1;
     ay=mouse.y-father.y;
     father.alpha=200;
     signal(father,s_freeze);
     While (mouse.left)
      father.x=mouse.x-ax;
      father.y=mouse.y-ay;
      Frame;
     End;
     signal(father,s_wakeup);
     father.alpha=255;MouseLock=0;
    End;
   End; 
  End; 
 End;
Else
 // Subir;
 If (ToWinSel==father AND mouse.left and mLeftPress)
  UpIt(father);
 End;
End;
End;

Process UpIt(T);
Private proc,mxz,m,awinsel;
Begin
Proc=get_id(0);m=T.z;Mxz=m;
While (proc>0)
 If ((Proc.z<0) AND (proc.z>-500))
  If (Proc.z<mxz) mxz=Proc.z;End;
  If (m<>0 AND proc.z<m) proc.z++;End; 
 End;
 proc=get_id(0);
End;
If (M==0)
 T.z=mxz-1;
Else
 T.z=mxz;
End;
Winsel=T;
if (Winsel<>Awinsel)
 Winsel.msg=3;
 AWinsel=Winsel;
end;
End;

Process AddVBar(dc,x,y,al);
Private cha;
Begin
an=10;
cha=1;lotag=0;hitag=100;tag=0;
Loop
If (hitag<=lotag)
 If (cha<>hitag+10);
  drawing_map(0,dc);
  drawing_color(cBord);
  draw_rect(x+1,y+1,x+13,(y+al)-1);
  drawing_color(CBock);
  draw_box(x+1,y+1,x+13,(y+al)-1);
  father.msg=1;
  cha=hitag+10;
 End; 
Else
 If(cha<>tag)
  map_put(0,dc,513,x+7,y+7);
  map_xput(0,dc,513,x+7,y+al-7,0,100,2);
  drawing_map(0,dc);
  drawing_color(cBord);
  draw_line(x,y+14,x,y+al-14);
  draw_line(x+13,y+14,x+13,y+al-14);
  drawing_color(cBack);
  draw_box(x+1,y+14,x+13,y+al-14);
  map_put(0,dc,514,x+7,y+20+(tag*(al-40)/hitag));
  cha=tag;
  father.msg=1;
  Frame;
  father.msg=2;
  father.tag=id;
  Frame;
 End;
 If (mouse.left AND father==winsel);
  // say("gooo "+id+" --> "+winsel+" "+rand(0,1000));
  If (incuadre(mouse.x,mouse.y,x+father.x+father.ofsx,y+father.y+father.ofsy,14,14) AND tag>lotag)
   tag--;
  Else
   If (incuadre(mouse.x,mouse.y,x+father.x+father.ofsx,father.y+father.ofsy+y+al-14,14,14) AND tag<hitag)
    tag++;
   Else
    If (incuadre(mouse.x,mouse.y,x+father.x+father.ofsx,y+father.y+father.ofsy+19,14,al-40))
     Tag=((mouse.y-(y+father.y+father.ofsy+19))*hitag) / (al-40);
    End;
   End;
  End;  
 End;
End;
if (father.msg==256) break;end;
Frame;
End;
End;

Begin
End;
