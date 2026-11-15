program Lbdf;
private f,l,i,xx,yy,Num,des,has;
        String hex;
        String HexChars="0123456789ABCDEF";
begin
// l=Load_bdf("fnt.bdf");
graph=Load_map("MyFont.map");
set_center(0,graph,0,0);y=100;
f=Fopen("fnt.bdf",O_Write);
fputs(f,"STARTFONT 2.1");
// fputs(f,"FONT Fixed");
fputs(f,"COMMENT Generated with XWolf Map2Myfont (Lbdf.prg)");
fputs(F,"SIZE 8 200 200");
fputs(f,"FONTBOUNDINGBOX 6 8 0 0");
fputs(f,"STARTPROPERTIES 2");
fputs(f,"MinSpace 0");
fputs(f,"Copyright "" (C) XWolf '' """);
fputs(f,"ENDPPROPERTIES");
fputs(f,"CHARS "+itoa(256-32));
for (I=32;i<256;i++)
 hex=chr(I);
 if (I==32) hex="space";end;
 fputs (f,"STARTCHAR "+hex+" ");
 fputs (f,"ENCODING "+itoa(i));
 fputs (f,"SWIDTH 355 0");
 fputs (f,"DWIDTH 6 0");
 /*
 des=20;
 has=0;
 For (yy=0;yy<8;yy++);
  Num=0;
  for (xx=0;xx<6;xx++);
   if (map_get_Pixel(0,graph,(xx+(i*7))+1,yy+1)<>0)
    Num=1;
   end;
  end;
  if (num) has=yy;end
  if (num and des>yy) des=yy;end
 end;
 if (des==20)
  fputs (f,"BBX 0 0 0 0");
 else 
  fputs (f,"BBX 6 "+itoa(has)+" 0 0");
 end;
 */
 fputs (f,"BBX 6 8 0 0");
 fputs (f,"BITMAP");
 for (yy=0;yy<8;yy++);
  hex="";
  Num=0;
  For (xx=0;xx<6;xx++);
   if (map_get_pixel(0,graph,(xx+(i*7))+1,yy+1)<>0)
    num+=pow(2,6-xx);
   end;
  end;
  // say(itoa(yy)+","+itoa(num));
  if (num==0) hex="00";end
  while (num>0)
   Hex=HexChars[num mod 16]+hex;
   say(num+","+(num mod 16)+">"+hex);
   Num=Num / 16;
  end;
  if (len(hex)==1) hex="0"+hex;end;
  fputs (f,hex);
 end;
 fputs (f,"ENDCHAR");
end;
fputs(f,"ENDFONT");
fclose(f);
L=Load_bdf("Fnt.bdf");
Save_fnt(l,"Fixed.fnt");
write(l,0,0,0,"Hola!!!!1234567890 ][ {} .,:;-_çÇ!""·$%&/()=");
set_text_color(10);
write(l,0,20,0,"abcdefghijklmnñopqrstuvwxyz ABCDEFGHIJKLMNÑOPQRSTUVWXYZ");
save_fnt(l,"Console.fnt");
LOOP
if (key(_esc)) exit();end;
frame;
end;
end; 
