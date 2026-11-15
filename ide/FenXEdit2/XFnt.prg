// XFnt: Unidad de manejo de fuentes .fnt

process FNTBox(String dir,fil);
Private MP,ft;
Begin
IsWindow=1;
icon=103;
hint=dir+fil;
if (find(ucase(fil),".FNT")>0)
 Load_pal(dir+fil);
 ft=Load_fnt(dir+fil);
else
 ft=Load_bdf(dir+fil);
end;

Set_text_color(cText);
if (ft<0)
 ft=0;
 mp=Write_In_map(0," Error al leer fuente ",0);
else
 mp=Write_In_map(ft," ABC abc 123 ¥+-!@",0);
end;
an=graphic_Info(0,mp,g_width);
al=graphic_info(0,mp,g_height)*2;
dc=New_dc(an,al);
AddImage(dc,0,al / 4,mp);
Unload_map(0,mp);
graph=New_Window(an+13,al+28,Fil,0,dc);
x=rand(10,50);
y=rand(20,150);
upit(id);
for (alpha=0;alpha<255;alpha+=16);frame;end;alpha=255;
loop
if (msg==2)
 break;
end;
WinProc();
frame;
end;
msg=256;
for (alpha=255;alpha>0;alpha-=16);frame;end;
unload_map(0,graph);
unload_map(0,dc);
if (ft<>0)
 Unload_fnt(ft);
end;
end;

Begin
end; 
