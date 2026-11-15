program tfntdiv;
private mp,an,al,c,pp;
begin
mp=load_map("D:\div\xwolf\fntx.map");
an=graphic_info(0,mp,g_Wide);
al=graphic_info(0,mp,g_height);
pp=Mapi(mp);
for (X=0;x<an;x++);
 for (y=0;y<al;y++);
  c=map_get_pixel(0,mp,x,y);
  if (c==22) c=0;end;
  map_put_pixel(0,mp,x,y,c);
  frame(1);
 end;
 pp.x=160-x;
end;
Save_map(0,mp,"nf.map");
loop
frame;
end;
end;

process mapi(graph);
begin
loop
frame;
end;
end;
