Program Fonedit;
Private c,vx,vy;
        String chrc;
Begin
/*For (x=0;x<255;x++);
 say(itoa(x)+":"+chr(x));
End;*/
restore_type=1;
Dump_type=1;
mouse.graph=new_map(24,24,8);
drawing_map(0,mouse.graph);
set_center(0,mouse.graph,0,0);
For (x=0;x<24;x++)
 drawing_color((x/2)+3);
 draw_line(0,x,x,x);
End;
set_title("FenXEdit ''MyFont'' editor");
set_mode(250,360,16);
graph=load_map("MyFont.map");
set_center(0,graph,0,0);
size=3000;
write(0,0,300,0,"Caracter:    (     )");
write_var(0,60,300,0,chrc);
write_int(0,80,300,0,&c);
// Write_var(0,0,310,0,ascii);
x=0;FijaT(graph);FijaTBien(graph);
Loop
chrc=chr(c);
if (mouse.y<300)
 If (mouse.wheelUp)   c--;End;
 If (mouse.wheeldown) c++;End;
end;
x=-(c*210);
If (ascii<>0) c=ascii;End;
vx=mouse.x/30;
vy=mouse.y/30;
If ((vx>0 AND vx<7) AND (vy>0 AND vy<9))
 If (mouse.left)
  map_put_pixel(0,graph,vx+(c*7),vy,15);
 End;
 If (mouse.Right)
  map_put_pixel(0,graph,vx+(c*7),vy,0);
 End; 
End;
If (key(_enter))
 save_map(0,graph,"MyFont.Map");
End;
If (key(_esc))
 exit();
End;
Frame;
End;
End; 

Process FijaT(Graph);
private c;
begin
y=310;
size=300;
loop
x=-(c*50);
if (mouse.y>300 and mouse.y<342)
 if (mouse.wheelup)   c--;end;
 if (mouse.wheeldown) c++;end;
end;
frame;
end;
end;

process FijaTBien(graph);
private c;
begin
y=345;
loop
x=-(c*70);
if (Mouse.y>342)
 if (mouse.wheelUp) c--;end;
 if (mouse.wheeldown) c++;end;
end;
frame;
end;
end;
