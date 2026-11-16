// XDir: Unidad de manejo de directorios
Global
 String Infox[]="Desconocido","Directorio anterior","Directorio","Programa f‚nix",
  "Sonido/M£sica","Fichero de gr ficos","C¢digo Div/F‚nix","Proyecto FenXEdit ][",
  "C¢digo Comprimido","Fuente Div/F‚nix","Im gen","Sonido Div (PCM)";


// Ventana principal de navegacion
Process DirWin(String hint);
Private bar;
        Struct Fil[1800]
         String Name,Date,Siz;
         Int Info;
        End;
        String str,ext;
        I,ia,J,K,maxfil,filsel,afilsel,proc,TimToDo;
        MiSong,Ruta;
        String MenuDat="Nuevo c¢digo (.PRG)
Nuevo mapa (.MAP)
Nuevo fichero (.FPG)
Nueva fuente (.FNT)
Nuevo direcotrio
-
Actualizar";
Begin
x=rand(1,50);
y=rand(50,70);
IsWindow=1;
an=305;
al=295;
Lastbar(&Hint);
dc=New_Dc(an,al);
bar=AddVBar(dc,an-13,-1,al+2);
Ruta=AddText(dc,-1,-1,an-11);
Control=ruta;
graph=New_Window(an+13,al+28,hint,0,dc);
Icon=100;
UpIt(Id);
For (alpha=0;alpha<255;alpha+=24);Frame;End;alpha=255;
Msg=MSG_RELOAD;maxfil=-1;filsel=-1;afilsel=-2;
if (hint[0]==".")
 hint="/";
end;
ruta.hint=hint;
Loop
if (msg==MSG_MODIFY)
 if ((Glob(Ruta.hint)<>"" and Fileinfo.Directory) or ((Len(Ruta.hint)==3 and Ruta.hint[1]==":") or (Ruta.hint=="/")))
  str=Ruta.Hint;
  Lastbar(&str);
  if (Str<>Hint)
   hint=str;     
   msg=MSG_RELOAD;
  end;
 end;
end;
If (Msg==MSG_DROP)
 Switch (Drag.msg)
  Case DRAG_MAP:
   Proc=InputBox("Guardar Mapa","Nombre de archivo",drag.Hint);frame;
   if (Proc.hint<>"")
    If (key(_Control))
     Extensa(&Proc.hint,".png");
     Save_PNG(Drag.hitag,Drag.tag,hint+Proc.hint);
    Else
     Extensa(&proc.hint,".map");
     Save_MAP(Drag.hitag,Drag.tag,hint+Proc.hint);
    End;
    Msg=MSG_RELOAD;
   end;
  end;
 end;
end;
If (msg==MSG_RELOAD)
 ext="";
 chdir(hint);
 Proc=0;
 While ((Str=glob("*.*"))<>"")
  If (str<>".")
   i=0;
   While (i<Proc and Ucase(str)>ucase(fil[i].name) and str<>"..")
    i++;
   end;
   j=Proc;
   while (j>=i)
    Fil[j+1].name=Fil[j].name;
    Fil[j+1].date=Fil[j].date;
    Fil[j+1].siz =Fil[j].siz ;
    Fil[j+1].Info=Fil[j].Info;
    j--;
   end;
   Proc++;
   Fil[i].Name=Fileinfo.name;
   Fil[i].Date=Fileinfo.modified;
   ext="B";
   K=Fileinfo.size;
   If (k>1024) K=K / 1024;Ext="K";End;
   If (K>1024) K=K / 1024;Ext="M";End;
   If (K>1024) K=K / 1024;Ext="G";End;
   If (k>1024) k=k / 1024;Ext="T";End;
   Fil[i].siz=itoa(k)+" "+ext;
   ext=ucase(substr(Str,find(str,"."),len(str)));
   If (Fileinfo.directory)
    If (str=="..")
     Fil[i].info=1;
    Else
     FIl[i].Info=2;
    End;
   Else
    Fil[i].info=0;
    If (ext==".DCB")
     Fil[i].info=3;
    Else
     If (ext==".WAV" OR ext==".MID" OR ext==".MOD" OR ext==".IT" OR ext==".XM" OR ext==".S3M" OR ext==".OGG") 
      Fil[i].info=4;
     Else
      If (ext==".FPG")
       Fil[i].info=5;
      Else
       If (ext==".PRG");
        Fil[i].Info=6;
       Else
        If (ext==".FXP")
         Fil[i].Info=7;
        Else
         If (ext==".PRZ")
          Fil[i].Info=8;
         Else
          If (ext==".FNT" or ext==".BDF")
           Fil[i].info=9;
          Else
           If (ext==".MAP" OR ext==".PNG" OR ext==".PCX" or ext==".BMP" OR EXT==".JPG" OR EXT==".JPE" or ext==".JPEG" or ext==".LBM" or ext==".XPM" or ext==".XBM" or ext==".GIF" or ext==".TIF" or ext==".TIFF" or ext==".XCF" or ext==".PNM")
            Fil[i].info=10;
           Else
            If (ext==".PCM")
             Fil[i].info=11;
            End; // PCM
           End; // MAP
          End; // FNT
         End; // PRZ
        End; // FXP
       End; // PRG
      End; // FPG
     End; // WAV
    End; // DCB
    /*
    Switch (ext) 
     Case ".DCB": 
      Fil[i].info=3;
     End 
     Case ".WAV",".MID",".MOD",".IT",".XM",".S3M",".OGG":
      Fil[i].info=4;
     End 
     Case ".FPG":
      Fil[i].info=5;
     End 
     Case ".PRG":
      Fil[i].info=6;
     End;
     Case ".FXP":
      Fil[i].info=7;
     End;
     Case ".PRZ":
      Fil[i].info=8;
     End;
     Case ".FNT",".BDF":
      Fil[i].info=9;
     End;
     Case ".MAP",".PNG",".PCX",".JPG",".JPE",".JPEG",".PNG",".GIF",".BMP",".TIF",".TIFF",".LBM",".XPM",".XCF",".PNM":
      Fil[i].info=10;
     End;
     Case ".PCM":
      Fil[i].info=11;
     End;
    End; //Switch
    */ 
    // Proc++;
   End; //FileInfo=Directory
  End; // Str<>"."
 End; // Str<>""
 Maxfil=proc;
 bar.hitag=maxfil-15;
 Msg=MSG_REDRAW;
 afilsel=-2;
 Filsel=0;
 Bar.tag=0;
End; // Msg==MSG_RELOAD

If (msg==MSG_CLICK)
 If (tag==1) Break;End;
 If (tag==bar) msg=MSG_REDRAW;End;
End;
If (msg==MSG_REDRAW)
 unload_map(0,graph);
 drawing_map(0,dc);ia=-1;
 For (i=0;(i<15) AND (ia<Maxfil-1);i++)
  ia=i+bar.tag;
  If ((i+bar.tag)==filsel)
   J=cSele;
   K=cBack;
  Else
   J=cBock;
   K=cText;
  End;
  // ----------------------------------------------------------------------------
  drawing_color(j);
  draw_box(0,(i*19)+11,20,(((i+1)*19)-1)+11);
  AddLabel2(dc,20,(i*19)+11,fil[ia].Name,K,j,150,8,0);
  map_put(0,dc,Fil[ia].info+200,9,(i*19)+20);
  Addlabel2(dc,20,(i*19)+19,infox[fil[ia].info],cBord,cBeck,148,8,2);
  Addlabel2(dc,170,(i*19)+11,Fil[ia].date,k,j,122,8,1);
  Addlabel2(dc,168,(i*19)+19,fil[ia].siz+" ",cBord,cBeck,124,8,2);
 End;
 graph=New_Window(an,al,hint,0,dc);
 If (i<15)
  drawing_color(cBack);
  draw_box(0,i*17,292,294);// 295,254
 End;
End;
WinProc();
If (ToWinSel==id)
 If (mouse.wheelup AND Bar.tag>bar.lotag)
  bar.tag--;
  If (bar.tag>bar.lotag)bar.tag--;End;
 End;
 If (mouse.wheeldown AND bar.tag<bar.hitag);
  bar.tag++;
  If (bar.tag<bar.hitag)bar.tag++;End;
 End;
End; 
If (Winsel==id AND Incuadre(mouse.x,mouse.y,x+ofsx+11,y+ofsy,an-36,al-28)) 
 filsel=((mouse.y-(y+ofsy+11)) / 19)+bar.tag;
 TimToDo++;
 If (filsel>=maxfil)filsel=-1;End;
 If (Filsel<>aFilsel) 
  Msg=MSG_REDRAW;
  TimToDo=0;
  afilsel=filsel;
  /*
  if (!Drag.DC)
   Drag.icon=Fil[FilSel].info+200;
   Drag.Control=id;
   Drag.Hint=Fil[FilSel].name;
   Drag.Msg=MSG_REDRAW;
  end;
  */
  if (misong>0)
   Stop_song();
   Unload_song(Misong);
   Misong=0;
  end;
 End;
 If (TimToDo>30)
  if (misong==0 and !SiMusica and Fil[filsel].info==4)
   Misong=Load_song(Hint+fil[filsel].name);
   play_song(Misong,0);
  end;
 End;
 If (mousedblclick AND (filsel<>-1))
  While(mouse.left);Frame;End;
  Switch (Fil[filsel].info)
   Case 1:
    Updir(&hint);
    Ruta.hint=hint;
    frame;
    Msg=MSG_RELOAD;
   End;
   Case 2:
    Str=Fil[Filsel].name;
    Hint=Hint+str+"/";
    Ruta.Hint=hint;
    frame;
    Msg=MSG_RELOAD;
   End;
   Case 3: // Bytecode Fenix Compilado (.DCB)
   end;
   Case 4: // Sonido/M£sica
    if (MiSong>0)
     Stop_song();
     Unload_song(Misong);
     MiSong=0;
    end;
    SoundBox(hint+Fil[FilSel].name);
   End;
   Case 5: // Ficheros FPG
    FPGBox(hint,Fil[FilSel].name);
   end;
   Case 6: // C¢digo
    OpenText(hint,Fil[FilSel].name);
   End;
   Case 9: // Fuentes
    FNTBox(hint,Fil[FilSel].name);
   end;
   Case 10: // Mapas
    OpenMap(hint,Fil[FilSel].name);
   End;
   Case 11: // PCM
    Pcm2Wav(hint+Fil[Filsel].name);
    msg=MSG_RELOAD;
   end;
  End;
 End;
 If (mouse.right)
  I=Menu(mouse.x,mouse.y,MenuDat);Frame;
  // say(i.tag);
  switch (i.tag)
   Case 0:  // codigo
    Proc=InputBox("Nuevo c¢digo","Nombre del prg","");frame;
    if (proc.hint<>"")
     str=Proc.hint;
     If (find(str,".")<0)
      extensa(&Str,".prg");
     end;
     If (File_Exists(str))
      Proc=SiNoBox("Nuevo c¢digo","El fichero ya existe ¨Continuar?");frame;
      j=Proc.tag;
     else
      j=1;
     end;
     if (J)
      Proc=Fopen(hint+Str,o_Write);
       fputs(Proc,"// -------------------------------------");
       fputs(Proc,"// Programa:    "+str+"");
       fputs(Proc,"// Autor:       ");
       fputs(Proc,"// Comentarios: ");
       fputs(Proc,"// IDE: "+appname+" Version:"+appversion+" [Http://www.iespana.es/Semitex/fenix/]");
       fputs(Proc,"// -------------------------------------");
       fputs(Proc,"");
       fputs(Proc,"Program "+substr(str,0,-4)+";");
       fputs(Proc,"Begin");
       fputs(proc,"");
       fputs(Proc," Loop");
       fputs(proc,"  if (key(_esc)) exit();end;");
       fputs(Proc,"  Frame;");
       fputs(Proc," End;");
       fputs(Proc,"End;");
      FClose(Proc);
     end;
     OpenText(Hint,str);
    end;
   end;
   Case 1:  // Mapa
    //NewMapBox();
   end;
   Case 2:  // fpg
    //NewFpgBox();
   end;
   Case 4:  //directorio
    Proc=InputBox("Nuevo direcotrio:","Nombre del direcotrio","");frame;
    if (Proc.hint<>"")
     if (mkdir(hint+Proc.hint)<0)
      MessageBox("Nuevo Directorio:","No se pudo crear el directorio!!");frame;
     end;
    end;
    msg=MSG_RELOAD;
   end;
   Case 6:  // recargar
    msg=MSG_RELOAD;
   end; 
  end;
 End;
else
 TimToDo=0;
 If (misong>0)
  Stop_song();
  Unload_song(misong);
  Misong=0;
 end;
End;
Frame;
End;
msg=MSG_CLOSING;
For (alpha=255;alpha>0;alpha-=24);Frame;End;alpha=0;
unload_map(0,graph);
unload_map(0,dc);
End;

// la £ltima barra si no la tiene
Process LastBar(String Pointer Str);
Private String TheLast;
Begin
 TheLast=substr([Str],len([Str])-1,1);
 If (NOT (TheLast==chr(92) OR TheLast=="/")) [Str]=[Str]+"/";End;
End;
                    
// la extension si no la tiene
Process Extensa(String pointer str,string ext);
begin
 if (find([str],".")==-1);
  [str]+=ext;
 end;
end;

// la extension
Process GetExt(String Str,String Pointer Ext);
begin
[ext]="";
return;
 if (find(str,".")==-1)
  [Ext]="";
 Else
  x=Len(str);
  while (str[x]<>".") x--;end;
  [ext]=substr(str,x);
 end;
end;

// cambia la extension
Process SetExt(String Pointer str,String ext);
begin
 if (find([str],".")==-1)
  [str]+=ext;
 else
  x=len([str]);
  while ([str][x]<>".") x--;end;
  [str]=Substr([str],0,x)+ext;
 end;
end;

// Directorio anterior
Process UpDir(String Pointer str);
begin
 if ([str]=="/" or ([str][1]==":" and [str][2]==chr(92) and len([str])==3))
  return (0);
 end;
 x=len([str])-2;
 while (x>0 and [str][x]<>"/" and [str][x]<>chr(92))
  x--;
 end;
 [str]=substr([str],0,x+1);
 return (1);
end;
