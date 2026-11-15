Program fnt3;
Private mp,an,al,as,aas,cnt,leko;
begin
mp=Load_map("Xwolf\Fntx.map");
leko=Bisu(0,0,mp);
Put_screen(0,load_map("xwolf\bkkg.map"));
an=graphic_info(0,mp,g_Wide);
al=graphic_info(0,mp,g_height);
Write_int(0,161,15,0,&cnt);cnt=0;
// 242
for (X=0;x<an;x++);
 as=0;
 for (y=0;y<al;y++);
  if (Map_get_Pixel(0,mp,x,y)<>242) as=1;end;
//  frame(1);
 end;
 if (as==1 and aas==0)
  cnt++;
 end;
 Leko.x=160-x;
 frame(20);
 //pp.x=160-x;
 aas=as;
end;

loop
frame;
end;

end;

Process Bisu(x,y,graph);
begin
loop
frame;
end;
end;
