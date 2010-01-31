/*
  LICENSE
  -------
Copyright 2005 Nullsoft, Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer. 

  * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution. 

  * Neither the name of Nullsoft nor the names of its contributors may be used to 
    endorse or promote products derived from this software without specific prior written permission. 
 
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR 
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND 
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR 
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT 
OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/
#ifndef _R_DEFS_H_
#define _R_DEFS_H_

#include "config.h"
#include "avs_globals.h"

// inlines
static int __inline max(x, y)
{
    return x > y ? x : y;
}

static int __inline min(x, y)
{
    return x < y ? x : y;
}

static unsigned int __inline BLEND(unsigned int a, unsigned int b)
{
	register unsigned int r,t;
	r=(a&0xff)+(b&0xff);
	t=min(r,0xff);
	r=(a&0xff00)+(b&0xff00);
	t|=min(r,0xff00);
	r=(a&0xff0000)+(b&0xff0000);
	t|=min(r,0xff0000);
	r=(a&0xff000000)+(b&0xff000000);
	return t|min(r,0xff000000);
}

#if 1
#define FASTMAX(x,y) max(x,y)
// (x-(((x-y)>>(32-1))&(x-y))) // hmm not faster :(
#define FASTMIN(x,y) min(x,y)
//(x+(((y-x)>>(32-1))&(y-x))) 
#else
#pragma warning( push, 1 )

static __inline int FASTMAX(int x, int y)
{
  __asm
  {
    mov ecx, [x]
    mov eax, [y]
    sub	ecx, eax
  	cmc
  	and	ecx, edx
  	add	eax, ecx
  }
}
static __inline int FASTMIN(int x, int y)
{
  __asm
  {
    mov ecx, [x]
    mov eax, [y]
    sub	ecx, eax
	  sbb	edx, edx
	  and	ecx, edx
	  add	eax, ecx
  }
}
#pragma warning( pop ) 

#endif

static unsigned int __inline BLEND_MAX(unsigned int a, unsigned int b)
{
	register unsigned int t;
  int _a=a&0xff;
  int _b=b&0xff;
	t=FASTMAX(_a,_b);
  _a=a&0xff00; _b=b&0xff00;
  t|=FASTMAX(_a,_b);
  _a=a&0xff0000; _b=b&0xff0000;
  t|=FASTMAX(_a,_b);
	return t;
}

static unsigned int __inline BLEND_MIN(unsigned int a, unsigned int b)
{
#if 1
	register unsigned int t;
  int _a=a&0xff;
  int _b=b&0xff;
	t=FASTMIN(_a,_b);
  _a=a&0xff00; _b=b&0xff00;
  t|=FASTMIN(_a,_b);
  _a=a&0xff0000; _b=b&0xff0000;
  t|=FASTMIN(_a,_b);
	return t;
#else
  __asm
  {
    mov ecx, [a]
    mov eax, [b]

    and ecx, 0xff
    and eax, 0xff

    mov esi, [a]
    mov ebx, [b]

    sub	ecx, eax
	  sbb	edx, edx

    and esi, 0xff00
    and ebx, 0xff00

	  and	ecx, edx
    sub	esi, ebx

	  sbb	edx, edx
	  add	eax, ecx

	  and	esi, edx
    mov ecx, [a]

	  add	ebx, esi
    and ecx, 0xff0000

    mov esi, [b]
    or eax, ebx

    and esi, 0xff0000

    sub	ecx, esi
	  sbb	edx, edx

	  and	ecx, edx
	  add	esi, ecx
    
    or eax, esi
  }
#endif
}

#ifdef FASTMAX
#undef FASTMAX
#undef FASTMIN
#endif


static unsigned int __inline BLEND_AVG(unsigned int a, unsigned int b)
{
	return ((a>>1)&~((1<<7)|(1<<15)|(1<<23)))+((b>>1)&~((1<<7)|(1<<15)|(1<<23)));
}


static unsigned int __inline BLEND_SUB(unsigned int a, unsigned int b)
{
	register int r,t;
	r=(a&0xff)-(b&0xff);
	t=max(r,0);
	r=(a&0xff00)-(b&0xff00);
	t|=max(r,0);
	r=(a&0xff0000)-(b&0xff0000);
	t|=max(r,0);
	r=(a&0xff000000)-(b&0xff000000);
	return t|max(r,0);
}

#ifdef NO_MMX
#define BLEND_ADJ BLEND_ADJ_NOMMX
#endif

static unsigned int __inline BLEND_ADJ_NOMMX(AvsGlobalProxy *obj, unsigned int a, unsigned int b, int v)
{
	register int t;
	t=obj->blendtable[a&0xFF][v]+obj->blendtable[b&0xFF][0xFF-v];
	t|=(obj->blendtable[(a&0xFF00)>>8][v]+obj->blendtable[(b&0xFF00)>>8][0xFF-v])<<8;
	t|=(obj->blendtable[(a&0xFF0000)>>16][v]+obj->blendtable[(b&0xFF0000)>>16][0xFF-v])<<16;
	return t;
}

static unsigned int __inline BLEND_MUL(AvsGlobalProxy *obj, unsigned int a, unsigned int b)
{
	register int t;
	t=obj->blendtable[a&0xFF][b&0xFF];
	t|=obj->blendtable[(a&0xFF00)>>8][(b&0xFF00)>>8]<<8;
  t|=obj->blendtable[(a&0xFF0000)>>16][(b&0xFF0000)>>16]<<16;
	return t;
}

static __inline void BLEND_LINE(AvsGlobalProxy *obj, uint32_t *fb, int color)
{
  register int bm=obj->line_blend_mode&0xff;
  switch (obj->line_blend_mode&0xff)
  {
    case 1: *fb=BLEND(*fb,color); break;
    case 2: *fb=BLEND_MAX(*fb,color); break;
    case 3: *fb=BLEND_AVG(*fb,color); break;
    case 4: *fb=BLEND_SUB(*fb,color); break;
    case 5: *fb=BLEND_SUB(color,*fb); break;
    case 6: *fb=BLEND_MUL(obj, *fb,color); break;
    case 7: *fb=BLEND_ADJ_NOMMX(obj, *fb,color,(obj->line_blend_mode>>8)&0xff); break;
    case 8: *fb=*fb^color; break;
    case 9: *fb=BLEND_MIN(*fb,color); break;
    default: *fb=color; break;
  }
}
// NOTE. WHEN USING THIS FUNCTION, BE SURE TO DO 'if (g_mmx_available) __asm emms;' before calling
// any fpu code, or before returning.
#pragma warning( push, 1 )

#ifndef NO_MMX
static unsigned int __inline BLEND_ADJ(unsigned int a, unsigned int b, int v)
{
  __asm
  {
    movd mm3, [v] // VVVVVVVV
    
    movd mm0, [a]
    packuswb mm3, mm3 // 0000HHVV

    movd mm1, [b]
    punpcklwd mm3, mm3 // HHVVHHVV
        
    movq mm4, [mmx_blend4_revn]
    punpckldq mm3, mm3 // HHVVHHVV HHVVHHVV

    punpcklbw mm0, [mmx_blend4_zero]
    pand mm3, [mmx_blendadj_mask]

    punpcklbw mm1, [mmx_blend4_zero]
    psubw mm4, mm3

    pmullw mm0, mm3      
    pmullw mm1, mm4
    
    paddw mm0, mm1

    psrlw mm0, 8
    
    packuswb mm0, mm0

    movd eax, mm0
  }
}
#endif


static __inline unsigned int BLEND4(AvsGlobalProxy *obj, unsigned int *p1, unsigned int w, int xp, int yp)
{
#ifdef NO_MMX
  register int t;
  unsigned char a1,a2,a3,a4;
  a1=obj->blendtable[255-xp][255-yp];
  a2=obj->blendtable[xp][255-yp];
  a3=obj->blendtable[255-xp][yp];
  a4=obj->blendtable[xp][yp];
  t=obj->blendtable[p1[0]&0xff][a1]+obj->blendtable[p1[1]&0xff][a2]+obj->blendtable[p1[w]&0xff][a3]+obj->blendtable[p1[w+1]&0xff][a4];
  t|=(obj->blendtable[(p1[0]>>8)&0xff][a1]+obj->blendtable[(p1[1]>>8)&0xff][a2]+obj->blendtable[(p1[w]>>8)&0xff][a3]+obj->blendtable[(p1[w+1]>>8)&0xff][a4])<<8;
  t|=(obj->blendtable[(p1[0]>>16)&0xff][a1]+obj->blendtable[(p1[1]>>16)&0xff][a2]+obj->blendtable[(p1[w]>>16)&0xff][a3]+obj->blendtable[(p1[w+1]>>16)&0xff][a4])<<16;
  return t;      
#else
  __asm
  {
    movd mm6, xp
    mov eax, p1

    movd mm7, yp
    mov esi, w

    movq mm4, mmx_blend4_revn
    punpcklwd mm6,mm6

    movq mm5, mmx_blend4_revn
    punpcklwd mm7,mm7

    movd mm0, [eax]
    punpckldq mm6,mm6

    movd mm1, [eax+4]
    punpckldq mm7,mm7

    movd mm2, [eax+esi*4]
    punpcklbw mm0, [mmx_blend4_zero]

    movd mm3, [eax+esi*4+4]
    psubw mm4, mm6

    punpcklbw mm1, [mmx_blend4_zero]
    pmullw mm0, mm4

    punpcklbw mm2, [mmx_blend4_zero]
    pmullw mm1, mm6

    punpcklbw mm3, [mmx_blend4_zero]
    psubw mm5, mm7

    pmullw mm2, mm4
    pmullw mm3, mm6

    paddw mm0, mm1
    // stall (mm0)

    psrlw mm0, 8
    // stall (waiting for mm3/mm2)

    paddw mm2, mm3
    pmullw mm0, mm5

    psrlw mm2, 8
    // stall (mm2)

    pmullw mm2, mm7
    // stall

    // stall (mm2)

    paddw mm0, mm2
    // stall

    psrlw mm0, 8
    // stall

    packuswb mm0, mm0
    // stall

    movd eax, mm0
  }
#endif
}


static __inline unsigned int BLEND4_16(AvsGlobalProxy *obj, unsigned int *p1, unsigned int w, int xp, int yp)
{
#ifdef NO_MMX
  register int t;
  unsigned char a1,a2,a3,a4;
  xp=(xp>>8)&0xff;
  yp=(yp>>8)&0xff;
  a1=obj->blendtable[255-xp][255-yp];
  a2=obj->blendtable[xp][255-yp];
  a3=obj->blendtable[255-xp][yp];
  a4=obj->blendtable[xp][yp];
  t=obj->blendtable[p1[0]&0xff][a1]+obj->blendtable[p1[1]&0xff][a2]+obj->blendtable[p1[w]&0xff][a3]+obj->blendtable[p1[w+1]&0xff][a4];
  t|=(obj->blendtable[(p1[0]>>8)&0xff][a1]+obj->blendtable[(p1[1]>>8)&0xff][a2]+obj->blendtable[(p1[w]>>8)&0xff][a3]+obj->blendtable[(p1[w+1]>>8)&0xff][a4])<<8;
  t|=(obj->blendtable[(p1[0]>>16)&0xff][a1]+obj->blendtable[(p1[1]>>16)&0xff][a2]+obj->blendtable[(p1[w]>>16)&0xff][a3]+obj->blendtable[(p1[w+1]>>16)&0xff][a4])<<16;
  return t;      
#else
  __asm
  {
    movd mm6, xp
    mov eax, p1

    movd mm7, yp
    mov esi, w

    movq mm4, mmx_blend4_revn
    psrlw mm6, 8

    movq mm5, mmx_blend4_revn
    psrlw mm7, 8

    movd mm0, [eax]
    punpcklwd mm6,mm6

    movd mm1, [eax+4]
    punpcklwd mm7,mm7

    movd mm2, [eax+esi*4]
    punpckldq mm6,mm6

    movd mm3, [eax+esi*4+4]
    punpckldq mm7,mm7

    punpcklbw mm0, [mmx_blend4_zero]
    psubw mm4, mm6

    punpcklbw mm1, [mmx_blend4_zero]
    pmullw mm0, mm4

    punpcklbw mm2, [mmx_blend4_zero]
    pmullw mm1, mm6

    punpcklbw mm3, [mmx_blend4_zero]
    psubw mm5, mm7

    pmullw mm2, mm4
    pmullw mm3, mm6

    paddw mm0, mm1
    // stall (mm0)

    psrlw mm0, 8
    // stall (waiting for mm3/mm2)

    paddw mm2, mm3
    pmullw mm0, mm5

    psrlw mm2, 8
    // stall (mm2)

    pmullw mm2, mm7
    // stall

    // stall (mm2)

    paddw mm0, mm2
    // stall

    psrlw mm0, 8
    // stall

    packuswb mm0, mm0
    // stall

    movd eax, mm0
  }
#endif
}


#pragma warning( pop ) 


static __inline void mmx_avgblend_block(int *output, int *input, int l)
{
#ifdef NO_MMX
  while (l--)
  {
    *output=BLEND_AVG(*input++,*output);
    output++;
  }
#else
  static int mask[2]=
  {
    ~((1<<7)|(1<<15)|(1<<23)),
      ~((1<<7)|(1<<15)|(1<<23))
  };
  __asm 
  {
    mov eax, input
    mov edi, output
    mov ecx, l
    shr ecx, 2
    align 16
mmx_avgblend_loop:
    movq mm0, [eax]
    movq mm1, [edi]
    psrlq mm0, 1
    movq mm2, [eax+8]
    psrlq mm1, 1
    movq mm3, [edi+8]
    psrlq mm2, 1
    pand mm0, [mask]
    psrlq mm3, 1
    pand mm1, [mask]
    pand mm2, [mask]
    paddusb mm0, mm1
    pand mm3, [mask]
    add eax, 16
    paddusb mm2, mm3

    movq [edi], mm0
    movq [edi+8], mm2

    add edi, 16

    dec ecx
    jnz mmx_avgblend_loop
    emms
  };
#endif
}


static __inline void mmx_addblend_block(int *output, int *input, int l)
{
#ifdef NO_MMX
  while (l--)
  {
    *output=BLEND(*input++,*output);
    output++;
  }
#else
  __asm 
  {
    mov eax, input
    mov edi, output
    mov ecx, l
    shr ecx, 2
    align 16
mmx_addblend_loop:
    movq mm0, [eax]
    movq mm1, [edi]
    movq mm2, [eax+8]
    movq mm3, [edi+8]
    paddusb mm0, mm1
    paddusb mm2, mm3
    add eax, 16

    movq [edi], mm0
    movq [edi+8], mm2

    add edi, 16

    dec ecx
    jnz mmx_addblend_loop
    emms
  };
#endif
}

static __inline void mmx_mulblend_block(AvsGlobalProxy *obj, int *output, int *input, int l)
{
#ifdef NO_MMX
  while (l--)
  {
    *output=BLEND_MUL(obj, *input++,*output);
    output++;
  }
#else
  __asm 
  {
    mov eax, input
    mov edi, output
    mov ecx, l
    shr ecx, 1
    align 16
mmx_mulblend_loop:
    movd mm0, [eax]
    movd mm1, [edi]
    movd mm2, [eax+4]
    punpcklbw mm0, [mmx_blend4_zero]
    movd mm3, [edi+4]
    punpcklbw mm1, [mmx_blend4_zero]
    punpcklbw mm2, [mmx_blend4_zero]
    pmullw mm0, mm1
    punpcklbw mm3, [mmx_blend4_zero]
    psrlw mm0, 8
    pmullw mm2, mm3
    packuswb mm0, mm0
    psrlw mm2, 8
    packuswb mm2, mm2
    add eax, 8

    movd [edi], mm0
    movd [edi+4], mm2

    add edi, 8

    dec ecx
    jnz mmx_mulblend_loop
    emms
  };
#endif
}

static void __inline mmx_adjblend_block(int *o, int *in1, int *in2, int len, int v)
{
#ifdef NO_MMX
  while (len--)
  {
    //*o++=BLEND_ADJ(*in1++,*in2++,inblendval);
  }
#else
  __asm
  {
    movd mm3, [v] // VVVVVVVV
    mov ecx, len

    packuswb mm3, mm3 // 0000HHVV
    mov edx, o

    punpcklwd mm3, mm3 // HHVVHHVV
    mov esi, in1

    movq mm4, [mmx_blend4_revn]
    punpckldq mm3, mm3 // HHVVHHVV HHVVHHVV
    
    pand mm3, [mmx_blendadj_mask]
    mov edi, in2

    shr ecx, 1
    psubw mm4, mm3
    
    align 16
_mmx_adjblend_loop:
    
    movd mm0, [esi]
    
    movd mm1, [edi]
    punpcklbw mm0, [mmx_blend4_zero]
    
    movd mm6, [esi+4]
    punpcklbw mm1, [mmx_blend4_zero]
    
    movd mm7, [edi+4]        
    punpcklbw mm6, [mmx_blend4_zero]

    pmullw mm0, mm3
    punpcklbw mm7, [mmx_blend4_zero]

    pmullw mm1, mm4
    pmullw mm6, mm3

    pmullw mm7, mm4
    paddw mm0, mm1

    paddw mm6, mm7
    add edi, 8

    psrlw mm0, 8
    add esi, 8

    psrlw mm6, 8
    packuswb mm0, mm0
    
    packuswb mm6, mm6
    movd [edx], mm0

    movd [edx+4], mm6
    add edx, 8
    dec ecx
    jnz _mmx_adjblend_loop

    emms
  };
#endif
}

#endif
