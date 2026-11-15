Program fnt2;
Private mp,an,al,as,aas,cnt;
begin
mp=Load_map("Xwolf\Fntx.map");
an=graphic_info(0,mp,g_Wide);
al=graphic_info(0,mp,g_height);
//Bisu(0,0,mp);
graph=New_map(an+7,al,0,0,0);
Map_put(0,graph,mp,0,0);
Save_map(0,graph,"Xwolf\Fntx2.map");
end;