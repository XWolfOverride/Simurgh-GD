// XHelp: unidad de ayuda
Const MaxXhsa=6;
Global
 XHSA[MaxXhsa]="HotKeys:",
        " F2:Guardar codigo",
        " F8:Ir a linea",
        " F9:Buscar proceso",
        "",                   //5
        "Contacto:";

Process XHelpBox(); // ayuda de FenXEdit
private scr=0;i;bar;
begin
 iswindow=1;
 an=320;
 al=200;
 dc=New_dc(an,al);
 bar=addvbar(dc,301,-1,202);
 graph=New_window(an+13,al+21,"Ayuda de "+AppName,0,dc);
 x=160;y=50;UpIt(id);
 for (alpha=0;alpha<255;alpha+=16); frame;end;alpha=255;
 msg=1;
 loop
  if (msg==2)
   break;
  end;
  if (msg==1)
   map_clear(0,dc,cback);
   for (i=0;i<MaxXhsa;i++)
    addlabel(dc,10,10+(i*10),XHSA[i]);
   end;
   unload_map(0,graph);
   Graph=New_window(an,al,"Ayuda de "+AppName,0,dc);
  end;
  WinProc();
  frame;
 end;
 msg=256;
 for (;alpha>0;alpha-=16);frame;end;
 unload_map(0,graph);
 unload_map(0,dc);
end;

Process HelpBox(); // Ayuda de fenix;
Begin
 isWindow=1;
 an=520;
 al=200;
 x=50;y=70;
 dc=New_dc(an,al);
 addlabel(dc,10,10,"Esperando libreria expot");
 Graph=New_window(an-13,al+21,"Ayuda de fenix",0,dc);
 UpIt(id);
 For (alpha=0;alpha<255;alpha+=16);frame;end;alpha=255;
 loop
  if (msg==2)
   break;
  end;
  winproc();
  Frame;
 end;
 msg=256;
 for (;alpha>0;alpha-=16);frame;end;
 Unload_Map(0,graph);
 Unload_Map(0,dc);
end;
