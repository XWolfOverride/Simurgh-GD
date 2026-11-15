// -------------------------------------
// Programa:   Binario a fenix
// Autor:      XWolf
// Comentarios:
// IDE: FenXEdit ][ Version:0.28 [Http://www.iespana.es/Semitex/fenix/]
// -------------------------------------

Program Nuevecito_y_sin_estrenar;
private in,out,lar,b;
        byte a;
        string s;
Begin
 // ¨Que tal si empiezas ya?
 set_fps(1000,0);
 in=Fopen("head.wav",o_read);
 out=Fopen("head.prg",o_Write);
 s="byte Head_wav[]=";
 lar=Flength(in);
 write_var(0,0,0,0,lar);
 while (lar-->0)
  fread(in,a);
  //HexByte(a,&s);
  s+=a+",";
  b++;
  if  (b>20)
   b=0;
   fputs(out,s);
   s="";
  end;
  frame;
 end;
 fputs(out,s);
 Fclose(out);
 Fclose(in);
End;

process HexByte(a,string pointer s);
Private string Hexc="0123456789ABCDEF";
begin
[s]=Hexc[a/16];
[s]+=hexc[a mod 16];
end;
