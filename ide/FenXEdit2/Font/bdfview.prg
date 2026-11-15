program bdffontview;
private fn;
begin
fn=Load_bdf("fnt.bdf");
write(fn,0,00,0,"abcdefghyjklmnñopqrstuvwxyz ¿?][}{;,:.|@#~€¬");
write(fn,0,20,0,"ABCDEFGHIJKLMNÑOPQRSTUVWXIZ - 1234567890");
loop
frame;
end;
end;
