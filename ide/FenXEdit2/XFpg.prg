// XFpg: Unidad de manejo de los archivos FPG

process FPGBox(String dir,fil);
private nfpg,i,ll,l,maxi,bar,mapsel,mp,gx,gy,color,csel,gyy,man,mal,por,amapsel,zMapSel;
        string s,Rinfo;
        awinsel,mapas,last=1,depth=0;
        String Menu="Nombre
N£mero
Duplicar
-
Borrar";
        Struct HotRect
         x,y,an,al;
        end;
Begin
csel=rgb(0,175,0);
IsWindow=1;
an=291;
al=210;
dc=New_dc(an,al);
graph=new_window(an+13,al+28,fil,0,dc);
x=rand(5,150);
y=rand(50,200);
UpIt(id);
for(alpha=0;alpha<255;alpha+=16);frame;end;alpha=255;
hint=dir+fil;
icon=104;msg=MSG_REDRAW;
nfpg=Load_fpg(hint);
bar=AddVBar(dc,278,-1,212);
For (mapas=i=0;i<1000;i++)
 if (map_exists(nfpg,i))
  mapas++;
  if (depth==0)
   depth=graphic_info(nfpg,i,g_depth);
  end;
 end;
end;
Bar.hitag=(mapas / 4)-2;
loop
aWinsel=Winsel;
if (Winsel==id)
 if (incuadre(mouse.x,mouse.y,x+5,y+12,an-24,al-24))
  Info=rinfo;
  Mapsel=(Mouse.x-(x+5)) / 70;
  Drag.PseudoID= !Drag.dc;
  If (mapsel>3) mapsel=3;end;
  Mapsel=mapsel+(((mouse.y-(y+21)) / 70)*4);
  if (MouseDblClick)
   while (mouse.left) frame;end;
   OpenFPGMap(nfpg,zMapSel,Rinfo);
  end;
  if (Mouse.wheelup   and bar.tag>0)        bar.tag--;end;
  if (mouse.wheeldown and bar.tag<bar.hitag)bar.tag++;end;
  if (Mouse.right)
   If (zMapSel>=0)
    Tag=Menu(mouse.x,mouse.y,Menu);frame;
    Color=tag.tag;
    switch (color)
     Case 0: // Nombre
      Tag=InputBox("Mapa: "+zMapSel,"Nombre del mapa:",Map_name(nfpg,zMapSel));frame;
      map_set_name(nfpg,zMapSel,Tag.hint);
     end;
     Case 1,2: // N£mero , Duplicar
      Tag=InputBox("Mapa: "+Map_Name(nfpg,zMapSel),"N£mero del mapa",zMapSel);frame;
      tag=atoi(tag.hint);
      If (Tag>0 and tag<1000)
       If (Map_Exists(nfpg,tag))
        Lotag=SiNoBox("Mapa: "+map_name(nfpg,zmapsel),"El mapa ya exsiste ¨Sobrescribir?");frame;
        Hitag=lotag.tag;
        if (hitag) Unload_map(nfpg,tag);end;
       Else
        Hitag=1;
       End; // exsiste
       If (Hitag)
        Fpg_add(nfpg,tag,nfpg,zMapSel);
        if (color==1)
         Unload_map(nfpg,zMapSel);
        else
         Mapas++;
        end; //color
       End; //hitag
      Else
       MessageBox("Error:","Rango entre 1 y 999");frame;
      End; // en rango
     End; // case
     Case 4: // Borrar
      tag=SiNoBox(Fil,"Borrar el Mapa "+zMapSel+"?");frame;
      if (tag.tag)
       Unload_map(nfpg,zMapSel);
       Mapas--;
      end;
     end;
    end; 
    Bar.hitag=(mapas/4)-2;
    If (Bar.tag>bar.hitag) bar.tag=bar.hitag;end;
    Msg=MSG_REDRAW;
    Save_fpg(nfpg,hint);
   end; // zMapSel>=0 
  end;
 end;
end;

if (amapsel<>Mapsel)
 msg=MSG_REDRAW;
 amapsel=mapsel;
end;

if (msg==MSG_CLICK)
 if (tag==1)
  break;
 end;
 if (tag=bar)
  msg=MSG_REDRAW;
 end;
end;
If (msg==MSG_DROP)
 If (Drag.msg==DRAG_MAP and Drag.control<>id)
  l=last;
  while (map_exists(nfpg,l)) l++;end;
  tag=InputBox("A¤adir mapa al fpg","N£mero del mapa:",l);frame;
  tag=atoi(tag.hint);
  If (tag>0 and tag<1000)
   if (Map_exists(nfpg,tag))
    Lotag=SiNoBox("Mapa: "+tag,"El mapa ya exsiste ¨Sobreescribir?");
    Hitag=Lotag.tag;
    If (Hitag) Unload_map(nfpg,tag);End;
   Else
    Hitag=1; 
   End; // exists
   If (HiTag)
    Fpg_add(nfpg,tag,Drag.hitag,Drag.tag);
    msg=MSG_REDRAW;mapas++;
    Save_fpg(nfpg,hint);
    bar.hitag=(mapas/4)-2;
    last=tag;
   End;
  Else
   MessageBox("Error:","Rango entre 1 y 999");Frame;
  End; // tag en el rango
 End; // DragInfo==2
End; //msg=MSG_DROP
if (msg==MSG_REDRAW)
 Unload_map(0,graph);
 Drawing_map(0,dc);
 Drawing_color(cBack);
 Draw_box(0,0,278,210);
 zMapSel=-1;
 for (ll=l=i=0;i<1000 and l<12;i++)
  If (map_exists(nfpg,i))
   gx=(l mod 4)*70;
   gy=(l / 4)*70;
   If (ll<bar.tag*4)
    ll++;
   else 
    mp=new_dc(68,20);
    s=itoa(i);
    while (len(s)<3)
     s="0"+s;
    end;
    AddLabel2(mp,0,0,s,cBack,cText,68,20,4);
    man=graphic_info(nfpg,i,g_width);
    mal=graphic_info(nfpg,i,g_height);
    maxi=max(man,mal);
    if (maxi>68)
     por=6800 / maxi;
    else
     por=100;
    end;
    gyy=gy+(25-((mal*por) / 200));
    if (gyy<gy) gyy=gy;end;
    Drawing_map(0,dc);
    if (l==mapsel)
     Color=cSele;
     Rinfo=Map_Name(nfpg,i)+" ["+man+"x"+mal+"x"+Graphic_info(nfpg,i,g_Depth)+"]";
     zMapSel=i;
     if (towinsel==id and !Drag.dc)
      Drag.Control=id;
      Drag.msg=DRAG_MAP;
      Drag.tag=i;
      Drag.hitag=nfpg;
      Drag.hint=Map_Name(Nfpg,i);
      Drag.PseudoID=0;
      Drag.Icon=104;
     end;
    else
     Color=cBock;
    end;
    Drawing_color(Color);
    draw_box(gx,gy,gx+68,gy+70);
    AddImage2(dc,gx+(34-((Man*por) / 200)),gyy,nfpg,i,por);
    Map_xput(0,dc,mp,gx,gy+50,0,100,4);
    Unload_map(0,mp);
    l++;
   end;
  end; 
 end;
 graph=new_window(an,al,fil+" ("+mapas+" mapas;"+depth+" bits)",0,dc);
end;
winproc();
frame;
end;
msg=MSG_CLOSING;
for (;alpha>0;alpha-=16);Frame;end;
Unload_map(0,graph);
Unload_map(0,dc);
end;

Begin
end; 
