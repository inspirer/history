#org 0x100
#output source\kernel\uncode.com

#shortdef code 15047

jmp start
#include $crt.ch
#include $file.ch
start:

// w:fopen(*fname,b:mode)           Open file          | 0 - Error, else Hdl
// w:fcreate(*fname,w:attrib)       Create file        |      --""--
// w:fclose(w:handle)               Close file         | 0 - Ok, else Error
// w:fread(w:handle,*buffer,w:num)  Read binary file   | Return num of bytes
// w:fwrite(w:handle,*buffer,w:num) Write binary file  |      --""--

   bx=fopen("kernel.bin",0);
   if (bx==0){
      printf("File not found\n")
      int 0x20
   }
   dx=fcreate("kernel.out",0);
   if (dx==0){
      printf("File not created\n")
      int 0x20
   }

   fread(bx,buffer,size_xor_table);
   fwrite(dx,start_xor_table,size_xor_table);
   cx=size_xor_table;si=buffer;di=start_xor_table
   { lodsb;ah=b:[di];xor al,ah;b:[di]=al;inc di;loop ?^0 }

   while(fread(bx,buffer,size_xor_table)!=0){
      cx=ax;push cx;di=buffer;si=start_xor_table
      { lodsb;ah=b:[di];xor al,ah;b:[di]=al;inc di;loop ?^0 }
      pop cx;fwrite(dx,buffer,cx);
   }
   fclose(dx);fclose(bx);
   printf("OK\n");
   bx=fcreate("_code_.bin",0);
   if(bx==0){ printf("code not created\n");int 0x20 }
   fwrite(bx,start_xor_table,size_xor_table);
   fclose(bx)
   int 0x20

#shortdef start_xor_table $

   db "CHAOS",26    // sign [6]
   db "KERNEL",[2]  // Driver name [8]
   dw 0x10          // Size in pages [2]

   clts;cli;nop;ax=0x20;es=ax;ds=ax;fs=ax;gs=ax;ss=ax
   esp=6431
   mov eax,0x80000001
   mov cr0,eax
   lldt [1578]
   callf d:cs:[18973]
   jmps 1564
   db 1,2,78,4,5,12,35,78,4
   db "kernel.bin",0

#shortdef size_xor_table $-start_xor_table

#data

buffer:
