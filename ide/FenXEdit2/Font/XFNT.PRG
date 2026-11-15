Program xfnt;
begin
x=Load_fnt("el.fnt");
write(x,0,00,0,"1234567890.¿?`*^ù¤¥");
write(x,0,20,0,"ABCDEFGHIJKLMN¥OPQRSTUVWXYZ");
write(x,0,40,0,"abcdefghijklmn¤opqrstuvwxyz");
loop
if (key(_esc)) exit("",0);end
frame;
end;
end;
