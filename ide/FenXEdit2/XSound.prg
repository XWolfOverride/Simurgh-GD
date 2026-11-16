// SoundBox     ventanas de sonido
Process SoundBox(String fisound);
Private 
 PlaysonB,StopsonB,sou=0,soun=0,sisoun=0;
 VolumB1;
 larfi,larmo,xtex=3;
 MoSoun=0,busoun;
 movite,aviso;

Begin
IsWindow=1;
busoun=find (ucase(fisound),".WAV");
If (busoun==-1) MoSoun=1;End
larfi=len(fisound);
larmo=larfi-26; 
larmo=larmo*8;
larmo=larmo-larmo*2;
sisoun=1;
If (MoSoun==0)
 sou=load_wav(fisound);
 soun=play_wav(sou,-1);
 set_wav_volume(soun,128);
 Icon=105;
Else
 If (SiMusica==0)
 sou=load_song(fisound);
   soun=play_song(sou,-1);
  set_song_volume(128);
  SiMusica=1;
  icon=105;
 Else
  sisoun=0;
  aviso=100;
  icon=102;
 End
End
hint=fisound;
//fisound=ucase(fisound);
x=160;
y=170;
dc=New_dc(200,61);
AddImage(dc,2,1,204);
AddLabel(dc,3,19,fisound);
PlaysonB=AddButton(dc,96,40,50,20,"Play");
// Playsonb.hint="Iniciar reproductor!!";
StopsonB=AddButton(dc,148,40,50,20,"Stop");
// Stopsonb.hint="Parar reproduccion";
VolumB1=AddVolBar(dc,5,45);
Addlabel2(dc,0,30,"  >>RePRoDuCieNDo<<",rgb(255,0,0),cback,300,8,0);
graph=new_window(213,89,"Reproductor (FenXEdit2)",0,dc);Hitag=0;UpIt(Id);
For (alpha=0;alpha<255;alpha+=16);Frame;End;alpha=255;
Loop
 If (aviso>0)
  aviso=aviso-1;
  addlabel2(dc,0,30,"  Solo se permite un Canal",ctext,cback,200,8,0);
  If (aviso<20)
   addlabel2(dc,0,30,"  Reproductor en StoP",ctext,cback,200,8,0);
  End
  unload_map(0,graph);
  graph=new_Window(213,89,"Reproductor (FenXEdit2)",0,dc);
 End
 If (larmo<0)
  If (movite==0)xtex=xtex-1;End
  If (movite==1)xtex=xtex+1;End
  If (xtex==1)movite=0;End
  If (xtex<larmo-6)movite=1;End
  Addlabel2(dc,xtex,19," "+fisound,ctext,cback,500,8,0);
  unload_map(0,graph);
  graph=new_Window(213,89,"Reproductor (FenXEdit2)",0,dc);
 End
 If ((msg==MSG_REDRAW) AND (sisoun==0) OR (larmo>-1))
  unload_map(0,graph);
  graph=new_Window(213,89,"Reproductor (FenXEdit2)",0,dc);
 End;
 If (msg==MSG_CLICK)
  If (tag==PlaysonB)
   addlabel2(dc,xtex,19,fisound,ctext,cback,198,8,0);
   If (sisoun==0)
    If (MoSoun==0)
     soun=play_wav(sou,-1);
     set_wav_volume(soun,volumB1.tag);
     Icon=105;
     sisoun=1;
    End
    If ((MoSoun==1) AND (SiMusica==0));
     sou=load_song(fisound); 
     Icon=105;
     soun=play_song(sou,-1);
     set_song_volume(volumB1.tag);
     sisoun=1;
     SiMusica=1;
    Else
     If (MoSoun==1)
      aviso=100;
     End;
    End;
    addlabel2(dc,0,30,"  >>RePRoDuCieNDo<<",rgb(255,0,0),cback,300,8,0);
   End;
   End;
   If (tag==StopsonB) 
    addlabel2(dc,0,30,"  Reproductor en StoP",ctext,cback,300,8,0);
    unload_map(0,graph);
    graph=new_Window(213,89,"Reproductor (FenXEdit2)",0,dc);
    If (sisoun==1)
     If (!MoSoun)
      stop_wav(soun);
     else
      stop_song();
      SiMusica=0;
      unload_song(sou);
     End
     Icon=102;
     sisoun=0;
    End
  End
  If (sisoun==1)
   If (MoSoun==0)
    If (tag==VolumB1)
     set_wav_volume(soun,volumB1.tag);
    End;
   End;
   If (MoSoun==1)
    If (tag==VolumB1)
     set_song_volume(volumB1.tag);
    End;
   End;
  End;
  If (tag==1) 
   If (sisoun==1)
    If (MoSoun==0) 
     stop_wav(soun);
    Else
     stop_song();
     SiMusica=0;
    End
   End
   If (MoSoun==0)
    unload_wav(sou);
   Else
    unload_song(sou);
   End;
   soun=0;sisoun=0;
   Break;
  End;
 End;
 Msg=MSG_NONE;
 Winproc();
 Frame;
End;
Msg=MSG_CLOSING;
For (alpha=255;alpha>0;alpha-=16);Frame;End;unload_map(0,graph);
unload_map(0,dc);
End;

Process Pcm2Wav(string fin);
private
 byte Head_wav[]=82,73,70,70,57,63,0,0,87,65,86,69,102,109,116,32,16,0,
                 0,0,1,0,1,0,248,42,0,0,248,42,0,0,1,0,8,0,100,97,116,97;
 Byte Buf[1024],BBuf;
 in,out,fle;
 string Fout;
begin
 fout=fin;
 setext(&fout,".wav");
 out=fopen(fout,o_write);
 fwrite(out,Head_wav);
 in=fopen(fin,o_read);
 fle=flength(in);
 fwrite(out,fle);
 while (fle>0)
  fread(in,bbuf);
  fwrite(out,bbuf);
  fle--;
 end;
 fclose(in);
 fclose(out);
end;
