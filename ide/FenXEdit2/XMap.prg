// XMap: Unidad de manejo de mapas

// Carga de mapas externa
process OpenMap(String dir,fil);
private nmap,string filu;
begin
 Filu=Ucase(fil);
 Load_pal(dir+fil);Frame; // Paleta
 If (find(filu,".MAP")>=0)
  nmap=load_map(dir+fil);
 Else
  If (Find(filu,".PCX")>=0)
   nmap=Load_pcx(dir+fil);
  Else
   If (find(filu,".PNG")>=0)
    nmap=Load_png(dir+fil);
   Else
    nmap=Load_image(dir+fil);
   End; //.png
  End; //.pcx
 End; //.map
 MapBox(fil,dir+fil,nmap);
End; 

// Carga de mapas interna
Process OpenFpgMap(nfpg,nmap,string text);
begin
nmap=Map_clone(nfpg,nmap);
MapBox(Map_Name(nfpg,nmap),text,nmap);
end;

// Ventana de mapas
process MapBox(String text,hint,int nmap);
private
 tabit,angrap,algrap,maxoc,tagrap=100;
 Prico,i,hit;
 string Textu;
begin
iswindow=1;
// info
tabit =graphic_info(0,nmap,G_Depth);
angrap=graphic_info(0,nmap,g_wide);
algrap=graphic_info(0,nmap,g_height);
Maxoc =Max(angrap,algrap);
// reduccion
if (Maxoc>400 or maxoc<20)
 tagrap=40000 / maxoc;
end
an=(angrap*tagrap) / 100;
al=(algrap*tagrap) / 100;
// Icono
Prico=3200 / maxoc;
Icon=New_map(32,32,16);
drawing_map(0,icon);
For (x=0;x<32;x+=5)
 For(y=0;y<32;y+=5)
  i=((x+y) mod 2)*100;
  Drawing_color(rgb(100+i,100+i,100+i));
  Draw_box(x,y,x+5,y+5);
 end;
end;
AddImage2(icon,16-((angrap*prico)/ 200),16-((algrap*prico)/ 200),0,nmap,Prico);
// Starting
dc=new_dc(an,al);
drawing_map(0,dc);
For (x=0;x<an;x+=9)
 For(y=0;y<al;y+=9)
  i=((x+y) mod 2)*100;
  Drawing_color(rgb(100+i,100+i,100+i));
  Draw_box(x,y,x+9,y+9);
 end;
end;
AddImage2(dc,0,0,0,nmap,tagrap);
graph=new_window(an+13,al+28,Text+" "+angrap+"x"+algrap+"x"+tabit+", "+tagrap+"%",0,dc);
x=rand(10,20);
y=rand(20,100);
UpIt(id);
for (alpha=0;alpha<255;alpha+=16);frame;end;alpha=255;
if (Find(text,".")==-1)
 Textu=text;
else
 Textu=Substr(text,0,Find(text,"."));
end;
// principal
loop
 if (msg==2)
  break;
 end
 hit=incuadre(mouse.x,mouse.y,x+ofsx,y+ofsy,an-13,al-28);
 if (ToWinSel==id and hit and !drag.DC)
  Drag.control=id;
  Drag.msg=2;
  Drag.tag=nmap;
  Drag.Hitag=0;
  Drag.hint=Text;
  Drag.PseudoID=Hit;
  Drag.Icon=icon;
 end;
 Winproc();
 frame;
end
// descarga
msg=256;
for (alpha=255;alpha>0;alpha-=16);frame;end;
Unload_map(0,graph);
Unload_map(0,dc);
Unload_map(0,icon);
Unload_map(0,nmap);
end;

Process DumpPal();
Private Struct Paleta[256]
         byte R,G,B;
        end; 
        i;
begin
get_colors(0,256,&paleta);
for (i=0;i<256;i++)
 say("R:"+paleta[i].r+" G:"+paleta[i].g+" B:"+paleta[i].b+" <"+i);
end;
end;
