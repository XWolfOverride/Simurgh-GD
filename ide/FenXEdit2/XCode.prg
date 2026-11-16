// XCode: Unidad de edicion de c¢digo
Global
 BlockMode=0;
// Abrir Archivo de texto
Process OpenText(String dir,fil);
Begin
if (cwins<maxcwins)
 Tag=-1;
 For (x=0;x<cWins;x++)
  if (dir==cWin[x].dir and fil==cWin[x].fil)
   tag=x;
  end;
 end;
 if (tag>=0)
  if (tag<>cwinsel)
   CloseCode();
   cWinsel=tag;
   OpenCode();
   lt.msg=MSG_REDRAW;
  end;
 else
  if (cWinsel>=0) CloseCode();end;
  cWin[cWins].dir=dir;
  cWin[cWins].fil=Fil;
  cWin[cWins].col=cWin[cWins].lin=0;
  cWin[cWins].sx =cWin[cWins].sy =0;
  cWinSel=cWins;
  OpenCode();
  cWins++;
  Lt.Msg=MSG_REDRAW;
 end;
else
 MessageBox(AppName,"No mas de "+itoa(maxCWins)+" ventanas");frame;//---------------------------
end;
End;

// Proceso de letritas
Process TextWin();
Private i,ii,a,Color,ini,fin;
        String str;
Begin
// Grafico letrico
graph=new_map(500,460,16); 
set_center(0,graph,0,0);
x=140;y=480;z=500;
Cur=Cursor();Msg=MSG_REDRAW;
Loop
While (y==480)
 Frame;
End;
If (ToWinsel==0)
 If (mouse.wheeldown)
  IF (Key(_control))
   cWin[cWinSel].sx+=10;
   Msg=MSG_REDRAW;
  Else
   cWin[cWinSel].sy+=10;
   Hitag=10;
   If (cWin[cWinSel].sy>Code.count) Hitag=10-(Code.count-cWin[cWinSel].sy);cWin[cWinSel].sy=(code.count);End;
   Tag=map_clone(0,graph);
   Msg=MSG_SCROLL_DOWN+46-hitag;
   Map_xput(0,graph,tag,0,-(10*hitag),0,100,128);
   Unload_map(0,tag);
  end;
 End;
 If (mouse.wheelup)
  If (key(_control))
   cWin[cWinSel].sx-=10;
   if (cWin[cWinSel].sx<0) cWin[cWinSel].sx=0;end;
   Msg=MSG_REDRAW;
  Else
   cWin[cWinSel].sy-=10;
   Hitag=10;
   If (cWin[cWinSel].sy<0) Hitag=10+cWin[cWinSel].sy;cWin[cWinSel].sy=0;End;
   Tag=Map_Clone(0,graph);
   Msg=MSG_SCROLL_UP+hitag;
   Map_xput(0,graph,tag,0,10*hitag,0,100,128);
   Unload_map(0,tag);
  end;
 End;
 IF (MOUSE.left and Mouse.graph==996)
  cWin[cWinSel].lin=((mouse.y-y) / 10)+cWin[cWinSel].sy;
  cWin[cWinSel].col=((mouse.x-185) / 6)+cWin[cWinSel].sx;
 end;
 If ((i=NextKey())>0)
  Switch (i)
   Case 7,8: // borrar suprimir
    if (!((cWin[cWinSel].lin==code.count and cWin[cWinSel].col==len(code.line[cWin[cWinSel].lin]) and i==7) or
        (cWin[cWinSel].lin==0 and cWin[cWinSel].col==0 and i==8))) 
     if (cWin[cWinSel].saved) cWin[cWinSel].saved=0;tb.msg=MSG_REDRAW;End;
     tb.msg=MSG_REDRAW;
     Msg=MSG_ROW+(cWin[cWinSel].lin-cWin[cWinSel].sy);
     if ((cWin[cWinSel].col==0 and i==8) or (cWin[cWinSel].col==len(code.line[cWin[cWinSel].lin])and i==7))
      if (cWin[cWinSel].col==0 and i==8)
       cWin[cWinSel].lin--;
       cWin[cWinSel].col=len(Code.line[cWin[cWinSel].lin]);
      end;
      Msg=MSG_SCROLL_DOWN+(cWin[cWinSel].lin-cWin[cWinSel].sy);
      Code.line[cWin[cWinSel].lin]+=Code.line[cWin[cWinSel].lin+1];
      for (a=cWin[cWinSel].lin+1;a<code.count;a++)
       Code.line[a]=code.line[a+1];
      end;
      Code.count--;
     else
      Delete(&Code.line[cWin[cWinSel].lin],cWin[cWinSel].col-(-8+i),1);
      if (I==8)
       cWin[cWinSel].col--;
      end;
     end;
    end;
   end;
   Case 9: // tab
    str=code.line[cWin[cWinSel].lin-1];
    if (cWin[cWinSel].col>=len(str))
     a=3;
    Else
     For (a=cWin[cWinSel].col;a<len(str) and !(str[a]==" " and str[a+1]<>" ");a++) end;
     a-=cWin[cWinSel].col;
     a++;
     if (a+cWin[cWinSel].Col>len(str))
      a=3;
     end;
    end;
    For (;a>0;a--)
     Insert(&code.line[cWin[cWinSel].lin],cWin[cWinSel].col," ");
     cWin[cWinSel].col++;
    end;
    Msg=MSG_ROW+(cWin[cWinSel].lin-cWin[cWinSel].sy);
   end;
   Case 13: // enter
    if (cWin[cWinSel].saved) cWin[cWinSel].saved=0;tb.msg=MSG_REDRAW;End;
    For (a=code.count;a>cWin[cWinSel].lin-1;a--)
     Code.line[a+1]=code.line[a];
    end;
    Code.count++;
    Msg=MSG_SCROLL_DOWN+(cWin[cWinSel].lin-cWin[cWinsel].sy);
    if (cWin[cWinSel].col==0)
     Code.line[cWin[cWinSel].lin]="";
    else
     if (cWin[cWinSel].col==len(code.line[cWin[cWinSel].lin]))
      Code.Line[cWin[cWinSel].lin+1]="";
     else
      Code.line[cWin[cWinSel].lin]  =SubStr(Code.line[cWin[cWinSel].lin]  ,0,cWin[cWinSel].col);
      Code.line[cWin[cWinsel].lin+1]=SubStr(Code.line[cWin[cWinSel].lin+1],cWin[cWinSel].col);
     end;
    end;
    cWin[cWinSel].lin++;
    For (a=0;a<len(code.line[cWin[cWinSel].lin-1]) and code.line[cWin[cWinSel].lin-1][a]==" ";a++)
     Code.line[cWin[cWinSel].lin]=" "+code.line[cWin[cWinSel].lin];
    end;
    cWin[cWinSel].col=a;
   end;
   Case 14: // arriva
    cWin[cWinSel].lin--;
   end;
   Case 15: // abajo
    cWin[cWinSel].lin++;
   end;
   Case 17:
    cWin[cWinSel].col--;
    if (cWin[cWinSel].col<0)
     if (cWin[cWinSel].lin==0)
      cWin[cWinSel].col=0;
     else
      cWin[cWinSel].lin--;
      cWin[cWinSel].col=len(Code.line[cWin[cWinSel].lin]);
     end;
    end;
   end;
   Case 16:
    cWin[cWinSel].col++;
   end;
   case 18: // Inicio
    For (a=0;a<len(code.line[cWin[cWinSel].lin]) and code.line[cWin[cWinSel].lin][a]==" ";a++);end;
    if (cWin[cWinSel].col==A)
     cWin[cWinSel].col=0;
    else
     cWin[cWinSel].col=A;
    end;
   end;
   case 19: // fin
    cWin[cWinSel].col=len(code.line[cWin[cWinSel].lin]);
   end;
   case 20: // PgUp
    cWin[cWinSel].lin-=20;
   end;
   case 21: // PgDn
    cWin[cWinSel].lin+=20;
   end;
   Case 32..255:
    if (cWin[cWinSel].saved) cWin[cWinSel].saved=0;tb.msg=MSG_REDRAW;End;
    if (cWin[cWinSel].col<len(Code.line[cWin[cWinSel].lin]))
     insert(&Code.line[cWin[cWinSel].lin],cWin[cWinSel].col,chr(i));
    else
     while (len(Code.line[cWin[cWinSel].lin])<cWin[cWinSel].col)
      Code.line[cWin[cWinSel].lin]+=" ";
     end;
     Code.line[cWin[cWinSel].lin]+=chr(i);
    end;
    cWin[cWinSel].col++;
    Msg=MSG_ROW+(cWin[cWinSel].lin-cWin[cWinSel].sy);
   end;
  end; 
  if (cWin[cWinSel].lin>Code.count)
   cWin[cWinSel].lin=code.count;
  end;
  if (cWin[cWinSel].lin<0)
   cWin[cWinSel].lin=0;
  end;
  if (cWin[cWinSel].lin<cWin[cWinSel].sy)
   msg=abs(cWin[cWinSel].lin-cWin[cWinSel].sy)-1;
   cWin[cWinSel].sy=cWin[cWinSel].lin;
   if (msg>40)
    Msg=MSG_REDRAW;
   else
    Tag=Map_clone(0,graph);
    Map_xput(0,graph,tag,0,(msg+1)*10,0,100,128);
    unload_map(0,tag);
    msg+=MSG_SCROLL_UP;
   end;
  end;
  if (cWin[cWinSel].lin>cWin[cWinSel].sy+45)
   msg=(45+((cWin[cWinSel].sy+45)-cWin[cWinSel].lin))+1;
   cWin[cWinSel].sy=cWin[cWinSel].lin-45;
   if (msg<1)
    msg=MSG_REDRAW;
   else
    tag=Map_clone(0,graph);
    Map_xput(0,graph,tag,0,((msg-46)*10),0,100,128);
    unload_map(0,tag);
    msg+=MSG_SCROLL_DOWN;
   end;
  end;
  if (cWin[cWinSel].col<cWin[cWinSel].sx)
   cWin[cWinSel].sx=cWin[cWinSel].col-2;
   if (cWin[cWinSel].sx<0) cWin[cWinSel].sx=0;end;
   msg=MSG_REDRAW;
  end;
  if (cWin[cWinSel].col>cWin[cWinSel].sx+74)
   cWin[cWinSel].sx=cWin[cWinSel].col-50;
   msg=MSG_REDRAW;
  end;
 end;
End;
If (towinsel==0 and cWinSel>=0)
 if (key(_f2))
  while (key(_f2)) frame;end;
  closeCode();
 end;
 If (key(_f8))
  a=InputBox("Ir a:","N£mero de linea?:",itoa(cWin[cWinSel].lin+1));frame;
  i=atoi(a.hint);
  if (i>Code.count) i=code.count;end;
  if (i<1) i=1;end;
  cWin[cWinSel].lin=i;
  cWin[cWinsel].sy=i;
  msg=MSG_REDRAW;
 end;
 if (key(_f9))
  a=ProcBox();frame;
 end;
end;
If (msg==MSG_REDRAW or Msg>=MSG_ROW)
 If (Code.count>=0);
  if (!UseCodeCut) Msg=MSG_REDRAW;end;
  Switch (msg)
   Case 1:
    Map_clear(0,graph,0);
    Ini=0;
    Fin=46;
   end;
   Case MSG_ROW..MSG_ROW+50:
    Ini=Msg-MSG_ROW;
    Fin=ini+1;
    Drawing_map(0,graph);
    Drawing_color(0);
    Draw_box(0,ini*10,500,(ini+1)*10);
   end;
   Case MSG_SCROLL_UP..MSG_SCROLL_UP+50:
    Ini=0;
    Fin=(Msg-MSG_SCROLL_UP)+1;
    Drawing_map(0,graph);
    Drawing_color(0);
    Draw_Box(0,0,500,(fin)*10);
   end;
   Case MSG_SCROLL_DOWN..MSG_SCROLL_DOWN+50:
    ini=msg-MSG_SCROLL_DOWN;
    Fin=46;
    Drawing_map(0,graph);
    Drawing_color(0);
    Draw_box(0,ini*10,500,460);
   end;
  end;
  For (i=ini;i<fin;i++);
   str=itoa(i+1+cwin[cwinsel].sy);
   Lz(&str);
   if (i+cwin[cwinsel].sy<=code.count)
    AddLabel2(graph,1,(i*10),str,cText,cBack,42,10,4);
    If (Usecolors)
     PutCodeLine (graph,45,(i*10)+1,Code.line[I+cwin[cwinsel].sy],cWin[cWinSel].col);
    else
     Set_text_Color(cReserved);
     PutAcodeLine(graph,45,(i*10)+1,Code.line[i+cWin[cWinSel].sy],cWin[cWinSel].col);
    end;
   end;
  End;
 End;
 Msg=MSG_NONE;
End;
Frame;
End;
End;

Process Lz(String Pointer str);
Begin
While(len([str])<5)
 [str]="0"+[str];
End;
End

Process OpenCode();
Private F,redaded,fsize;
        String S,cr,tab;
Begin
F=fopen(cWin[cWinSel].dir+cWin[cWinSel].fil,O_Read);Code.Count=0;
Code.Line[Code.Count]=fgets(F);
cr=chr(13);
tab=chr(9);
fsize=flength(f);
While (ftell(f)<fsize)
 Code.Count++;
 s=fgets(F);
 Code.line[code.count]="";
 For (x=0;x<len(s);x++)
  If (s[x]<>cr and s[x]<>tab)
   Code.line[code.count]+=s[x];
  end;
  if (s[x]==tab)
   Code.line[code.count]+="   ";
  end;
 end;
End;    
Fclose(F);
if (Code.count==0)
 Code.line[0]="";
 Code.Count++;
end;
cWin[cWinSel].Saved=1;tb.msg=MSG_REDRAW;
End;

Process CloseCode();
Private f;
        string fil;
Begin
if (cWinSel>=0 and !cWin[cWinSel].saved)
 Fil=cWin[cWinSel].dir+cWin[cWinSel].fil;
 say("Guardando: "+fil);
 F=fopen(fil,o_Write);
 for (x=0;x<=code.count;x++)
  fputs(f,Code.Line[x]);
 end;
 fclose(f);
 cWin[cWinSel].Saved=1;tb.msg=MSG_REDRAW;
end;
End;

Process Cursor();
Private uxy;
Begin
graph=4;
Loop
 If (father.y==20)
  graph=4;
  y=24+((cWin[cWinSel].lin-cWin[cWinSel].sy)*10);
  x=185+((cWin[cWinSel].col-cWin[cWinSel].sx)*6);
  uxy+=15000;
  alpha=128+(cos(Uxy)*127);
 Else
  graph=0;
 End;
 Frame;
End;
End;

// Proceso de pintar una linea---------------------------------------------
Process PutCodeLine(graph,x,y,String text,Int z);
Private mp,i,l,AT;
Begin
i=cWin[cWinSel].sx;
while (i<len(Text))
 l=0;
 at=Chartype(asc(Text[i]));
 while (i+l<len(text) and CharType(asc(Text[i+l]))==at)
  l++;
 end;
 Hint=substr(text,i,l);
 If (at<>0)
  Switch (at)
   Case 1:
    Set_text_color(WordColor(Hint));
   end;
   Case 2:
    Set_text_color(cSymbol);
   end;
   Case 3:
    if (Text[i+1]=="/")
     Set_Text_Color(cComment);
     L=Len(text)-i;
     Hint=Substr(text,i,l);
    else
     Set_Text_Color(cSymbol);
    end;
   end;
   Case 4:
    Set_Text_Color(cString);
   end;
  end;
  if (Hint=="""") Set_text_color(cString);end;
  mp=write_in_map(CF,Hint,0);
  map_put(0,graph,mp,x+((i-cWin[cWinSel].sx)*6),y);
  if (mp>0) unload_map(0,mp); end;
 end;
 i+=l;
End;
End;

Process PutACodeLine(Graph,x,y,String text,int z);
private mp;
begin
 If (cWin[cWinSel].sx==0)
  Mp=Write_In_Map(cf,text,0);
 Else
  If (Len(text)>cWin[cWinSel].sx)
   Mp=Write_in_map(cf,Substr(text,cWin[cWinSel].sx),0);
  Else
   Mp=0;
  End;
 End;
 If (Mp>0)
  Map_put(0,graph,mp,x,y);
  Unload_Map(0,Mp);
 end;
end;

process Insert (string pointer Str,int pos,string ch)
begin
 if (pos==len([str]))
  [str]+=ch;
 else
  if (pos==0)
   [str]=ch+[str];
  else
   [str]=Substr([str],0,pos)+ch+substr([str],pos);
  end;
 end;
end;
// 
Process Delete (String pointer str,Int pos,num);
begin;
 // pos: posicion (desde 1) del primer caracter a borrar
 if (len([str])<num+1)
  [str]="";
 else
  pos--;
  if (pos==0)
   [str]=Substr([str],num);
  else
   [str]=Substr([str],0,pos)+Substr([str],pos+num);
  end;
 end;
end;
