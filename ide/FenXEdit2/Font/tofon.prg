Program ToFon;
import "Image.dll";
private mp,f,i,c;
        string SS;
        an;
begin
set_mode(320,200,16);
an=(7*255)+1;
f=New_map(an,10,16);
map_clear(0,f,rgb(0,128,0));
set_center(0,f,0,0);
Mp=Load_image("..\imgs\TMF.bmp");
put_screen(0,mp);
/*
z=32;
for (x=0;x<8;x++)
 s="";
 for (y=0;y<40;y++)
  if (z<255)
   s+=chr(z);
   z++;
  end;
 end;
 say(S);
 frame;
end;
say("");
say(z);
*/
/*
For (X=0;x<100;x++)
 For (y=0;y<100;y++)
  c=Map_get_pixel(0,mp,x,y);
  if (c<rgb(128,128,128)) c=0;end;
  map_put_pixel(0,mp,x,y,c);
 end;
end;
*/
X=0;Y=3;
set_fps(80,0);
// say(chr(65));
write_int(0,160,12,1,&i);
Write_var(0,173,0,1,SS);
For (i=0;i<256;i++)
 Map_block_copy(0,f,1+(i*7),1,mp,X*6,Y*8,6,8,128);
 if (i>31) x++;end;
 if (X==40)
  X=0;
  y++;
 end;
 SS=chr(i);
 put_screen(0,mp);
 put(0,f,160-i*7,0);
 frame;
end;
for (x=0;x<an;x++)
 For (y=0;y<9;y++)
  c=map_get_Pixel(0,f,x,y);
  if (c==12680) c=0;end
  if (c==50746) c=rgb(255,255,255);end;
  if (c<>0) say(c);end
  map_put_pixel(0,f,x,y,c);
 end;
end;
put_screen(0,mp);
Put(0,f,0,0);
Save_map(0,f,"MyFont.map");
Write(0,160,100,4,"Correcto!!!!");
loop
frame;
end;
end;
