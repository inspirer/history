// ChaOS loader output functions

#shortdef NULL 0xFFFF

function WriteChar:(1)
{
  push bp;mov bp,sp
  push ax;push dx
  mov dl,b:ss:[bp+4];ah=2;int 0x21
  pop dx;pop ax
  pop bp
  ret 2
}

function WriteDecW:(1)
{
  push bp;mov bp,sp
  push bx;push cx;push dx;push ax;xor cx,cx
  mov ax,w:ss:[bp+4]
  while(ax!=0){
    xor dx,dx;mov bx,10;div bx
    add dl,'0';push dx;inc cx
  }if (cx==0){WriteChar('0')}
  while (cx>0){pop dx;WriteChar(dx);dec cx}
  pop ax;pop dx;pop cx;pop bx
  pop bp;ret 2
}

function WriteDecD:(1)
{
  push bp;mov bp,sp
  push ebx;push cx;push edx;push eax;xor cx,cx
  mov eax,d:ss:[bp+4]
  while(eax!=0){
    xor edx,edx;mov ebx,10;div ebx
    add dl,'0';push dx;inc cx
  }if (cx==0){WriteChar('0')}
  while (cx>0){pop dx;WriteChar(dx);dec cx}
  pop eax;pop edx;pop cx;pop ebx
  pop bp;ret 4
}

function WriteHexW:(1)
{
  push bp;mov bp,sp
  push cx;push dx;push ax;xor cx,cx
  mov ax,w:ss:[bp+4]
  while(cx<4){
    rol ax,4;mov dl,al;and dl,0F;add dl,'0'
    if(dl>'9'){add dl,7}
    WriteChar(dx)
    inc cx
  }pop ax;pop dx;pop cx
  pop bp;ret 2
}

function WriteHexD:(1)
{
  push bp;mov bp,sp
  push cx;push dx;push eax;xor cx,cx
  mov eax,d:ss:[bp+4]
  while(cx<8){
    rol eax,4;mov dl,al;and dl,0F;add dl,'0'
    if(dl>'9'){add dl,7}
    WriteChar(dx)
    inc cx
  }pop eax;pop dx;pop cx
  pop bp;ret 4
}

function WriteLN:(1)
{
  push bp;mov bp,sp;push si;push ax
  mov si,w:ss:[bp+4]
  if (si!=NULL){
    lodsb
    while(al!=0){
      WriteChar(ax)
      lodsb
    }
  }
  WriteChar(0xA);WriteChar(0xD);
  pop ax;pop si;pop bp;ret 2
}

function printf:()
{
  push bp;mov bp,sp;push si;push ax;push di
  mov si,w:ss:[bp+4];mov di,6;lodsb
  while(al!=0){
    ifelse (al=='\'){
      lodsb;if (al=='n'){writeln(null)}
      if (al=='w'){WriteDecW(w:[bp+di]);add di,2}
      if (al=='D'){WriteDecD(d:[bp+di]);add di,4}
      if (al=='h'){WriteHexW(w:[bp+di]);add di,2}
      if (al=='H'){WriteHexD(d:[bp+di]);add di,4}
      if (al!=0){lodsb}
    } else {
      WriteChar(ax);lodsb
    }
  }
  sub di,4;mov w:[@@printf_param+1],di;pop di;pop ax;pop si
  pop bp;@@printf_param:ret 2
}
