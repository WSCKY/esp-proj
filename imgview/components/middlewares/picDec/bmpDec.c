#include "bmpDec.h"
#include "string.h"
#include "esp_log.h"
#include "esp_heap_caps.h"

static const char *TAG = "BMP_DEC";

uint16_t ImgWidth = 0, ImgHeight = 0;

#define PARALLEL_LINES         1

void bmp_decode(const char *path, pDrawBitmap_t pDrawBitmap)
{
//	FIL* f_bmp;
//    uint16_t br;

    uint16_t count;
	uint8_t  rgb, color_byte;
	uint16_t x, y, color;
	uint16_t countpix = 0;

//	uint16_t realx = 0;
//	uint16_t realy = 0;
//	uint8_t  yok = 1;
//	uint8_t res;

	uint8_t *databuf = NULL;
	uint16_t readlen = BMP_DBUF_SIZE;
	uint16_t *lines = NULL;

//	uint8_t *bmpbuf;
	uint8_t biCompression = 0;

	uint16_t rowlen;
//	BITMAPINFO *pbmp;

	databuf = (uint8_t *)heap_caps_malloc(readlen, MALLOC_CAP_DMA);
	assert(databuf != NULL);

//	f_bmp=(FIL *)pic_memalloc(sizeof(FIL));
//	if(f_bmp==NULL)
//	{
//		pic_memfree(databuf);
//		return PIC_MEM_ERR;
//	}
	FILE *f = fopen(path, "r"); // read only.
	if(f == NULL) {
		ESP_LOGE(TAG, "can't open file %s", path);
	} else {
//	res = f_open(f_bmp, (const TCHAR *)filename, FA_READ);
//	if(res == 0) {
		readlen = fread(databuf, readlen, 1, f);
//		f_read(f_bmp, databuf, readlen, (UINT *)&br);
		BITMAPINFO *pbmp = (BITMAPINFO *)databuf;
		count = pbmp->bmfHeader.bfOffBits;
		color_byte = pbmp->bmiHeader.biBitCount >> 3;
		biCompression = pbmp->bmiHeader.biCompression;
		ImgHeight = pbmp->bmiHeader.biHeight;
		ImgWidth = pbmp->bmiHeader.biWidth;
//		ai_draw_init();

		if((ImgWidth * color_byte) % 4)
			rowlen = ((ImgWidth * color_byte) / 4 + 1) * 4;
		else
			rowlen = ImgWidth * color_byte;

		lines = (uint16_t *)heap_caps_malloc(ImgWidth * sizeof(uint16_t) * PARALLEL_LINES, MALLOC_CAP_DMA);
		assert(lines != NULL);

		color = 0;
		x = 0;
		y = 0;
		y = ImgHeight;
		rgb = 0;

//		realy = (y * picinfo.Div_Fac) >> 13;
//		bmpbuf = databuf;
		while(1) {
			while(count < readlen) {
				if(color_byte == 3) {
					switch (rgb) {
						case 0:
							color = databuf[count] >> 3;
							break;
						case 1: 	 
							color += ((uint16_t)databuf[count] << 3) & 0X07E0;
							break;
						case 2:
							color += ((uint16_t)databuf[count] << 8) & 0XF800;
							break;
					}
				} else if(color_byte == 2) {
					switch(rgb) {
						case 0:
							if(biCompression==BI_RGB) {//RGB:5,5,5
								color=((uint16_t)databuf[count]&0X1F);	 	//R
								color+=(((uint16_t)databuf[count])&0XE0)<<1; //G
							} else {//RGB:5,6,5
								color = databuf[count];                      //G,B
							}  
							break;
						case 1:
							if(biCompression == BI_RGB) {//RGB:5,5,5
								color += (uint16_t)databuf[count] << 9;  //R,G
							} else {                                    //RGB:5,6,5
								color += (uint16_t)databuf[count] << 8;	//R,G
							}
							break;
					}
				} else if(color_byte == 4) {
					switch (rgb) {
						case 0:				  
							color = databuf[count] >> 3; //B
							break;
						case 1: 	 
							color += ((uint16_t)databuf[count] << 3) & 0X07E0;//G
							break;	  
						case 2:
							color += ((uint16_t)databuf[count] << 8) & 0XF800;//R
							break;
						case 3:
							break;
					}
				} else if(color_byte == 1) {}

				rgb ++;
				count ++;
				if(rgb == color_byte) {
					if(x < ImgWidth) {
						lines[x] = color;// + y * ImgWidth
//						realx = (x*picinfo.Div_Fac)>>13;
//						if(is_element_ok(realx,realy,1)&&yok)
//						{
//							pic_phy.draw_point(realx+picinfo.S_XOFF,realy+picinfo.S_YOFF-1,color);
//						}
					}
					x ++;
					color = 0x00;
					rgb = 0;
				}
				countpix ++;
				if(countpix >= rowlen)
				{
//					y --;
//					if(y == 0) break;
//					realy=(y*picinfo.Div_Fac)>>13;
//					if(is_element_ok(realx,realy,0))yok=1;
//					else yok=0;
//					if((realy+picinfo.S_YOFF)==0)break;
					x = 0;
					countpix = 0;
					color = 0x00;
					rgb = 0;
//					if(y == PARALLEL_LINES) {
						// draw it.
						pDrawBitmap(x, y --, lines, ImgWidth, 1);
//					}
				}	 
			}
			readlen = fread(databuf, readlen, 1, f);
//			res=f_read(f_bmp,databuf,readlen,(UINT *)&br);
//			if(br!=readlen)readlen=br;
//			if(res||br==0)break;
//			bmpbuf = databuf;
			if(readlen == 0) break;
	 	 	count = 0;
		}
		fclose(f);
	}

	heap_caps_free(lines);
	heap_caps_free(databuf);

//	return res;
}		 

//uint8_t minibmp_decode(uint8_t *filename,uint16_t x,uint16_t y,uint16_t width,uint16_t height,uint16_t acolor,uint8_t mode)
//{
//	FIL* f_bmp;
//    uint16_t br;
//	uint8_t  color_byte;
//	uint16_t tx,ty,color;
//
//	uint8_t res;
//	uint16_t i,j;
//	uint8_t *databuf;
//	uint16_t readlen=BMP_DBUF_SIZE;
//
//	uint8_t *bmpbuf;
//	uint8_t biCompression=0;
//
//	uint16_t rowcnt;
//	uint16_t rowlen;
//	uint16_t rowpix=0;
//	uint8_t rowadd;
//
//	uint16_t tmp_color;
//
//	uint8_t alphabend=0xff;
//	uint8_t alphamode=mode>>6;
//	BITMAPINFO *pbmp;
//
//	picinfo.S_Height=height;
//	picinfo.S_Width=width;
//
//#if BMP_USE_MALLOC == 1
//	databuf=(uint8_t*)pic_memalloc(readlen);
//	if(databuf==NULL)return PIC_MEM_ERR;
//	f_bmp=(FIL *)pic_memalloc(sizeof(FIL));
//	if(f_bmp==NULL)
//	{
//		pic_memfree(databuf);
//		return PIC_MEM_ERR;
//	}
//#else
//	databuf=bmpreadbuf;
//	f_bmp=&f_bfile;
//#endif
//	res=f_open(f_bmp,(const TCHAR*)filename,FA_READ);
//	if(res==0)
//	{
//		f_read(f_bmp,databuf,sizeof(BITMAPINFO),(UINT*)&br);
//		pbmp=(BITMAPINFO*)databuf;
//		color_byte=pbmp->bmiHeader.biBitCount/8;
//		biCompression=pbmp->bmiHeader.biCompression;
//		picinfo.ImgHeight=pbmp->bmiHeader.biHeight;
//		picinfo.ImgWidth=pbmp->bmiHeader.biWidth;
//
//		if((picinfo.ImgWidth*color_byte)%4)rowlen=((picinfo.ImgWidth*color_byte)/4+1)*4;
//		else rowlen=picinfo.ImgWidth*color_byte;
//		rowadd=rowlen-picinfo.ImgWidth*color_byte;
//
//		color=0;
//		tx=0 ;
//		ty=picinfo.ImgHeight-1;
//		if(picinfo.ImgWidth<=picinfo.S_Width&&picinfo.ImgHeight<=picinfo.S_Height)
//		{
//			x+=(picinfo.S_Width-picinfo.ImgWidth)/2;
//			y+=(picinfo.S_Height-picinfo.ImgHeight)/2;
//			rowcnt=readlen/rowlen;
//			readlen=rowcnt*rowlen;
//			rowpix=picinfo.ImgWidth;
//			f_lseek(f_bmp,pbmp->bmfHeader.bfOffBits);
//			while(1)
//			{
//				res=f_read(f_bmp,databuf,readlen,(UINT *)&br);
//				bmpbuf=databuf;
//				if(br!=readlen)rowcnt=br/rowlen;
//				if(color_byte==3)
//				{
//					for(j=0;j<rowcnt;j++)
//					{
//						for(i=0;i<rowpix;i++)
//						{
//							color=(*bmpbuf++)>>3;		   		 	//B
//							color+=((uint16_t)(*bmpbuf++)<<3)&0X07E0;	//G
//							color+=(((uint16_t)*bmpbuf++)<<8)&0XF800;	//R
// 						 	pic_phy.draw_point(x+tx,y+ty,color);
//							tx++;
//						}
//						bmpbuf+=rowadd;
//						tx=0;
//						ty--;
//					}
//				}else if(color_byte==2)
//				{
//					for(j=0;j<rowcnt;j++)
//					{
//						if(biCompression==BI_RGB)//RGB:5,5,5
//						{
//							for(i=0;i<rowpix;i++)
//							{
//								color=((uint16_t)*bmpbuf&0X1F);			//R
//								color+=(((uint16_t)*bmpbuf++)&0XE0)<<1; 	//G
//		 						color+=((uint16_t)*bmpbuf++)<<9;  	    //R,G
//							    pic_phy.draw_point(x+tx,y+ty,color);
//								tx++;
//							}
//						}else  //RGB 565
//						{
//							for(i=0;i<rowpix;i++)
//							{
//								color=*bmpbuf++;  			//G,B
//		 						color+=((uint16_t)*bmpbuf++)<<8;	//R,G
//							  	pic_phy.draw_point(x+tx,y+ty,color);
//								tx++;
//							}
//						}
//						bmpbuf+=rowadd;
//						tx=0;
//						ty--;
//					}
//				}else if(color_byte==4)
//				{
//					for(j=0;j<rowcnt;j++)
//					{
//						for(i=0;i<rowpix;i++)
//						{
//							color=(*bmpbuf++)>>3;		   		 	//B
//							color+=((uint16_t)(*bmpbuf++)<<3)&0X07E0;	//G
//							color+=(((uint16_t)*bmpbuf++)<<8)&0XF800;	//R
//							alphabend=*bmpbuf++;
//							if(alphamode!=1)
//							{
//								tmp_color=pic_phy.read_point(x+tx,y+ty);
//							    if(alphamode==2)
//								{
//									tmp_color=piclib_alpha_blend(tmp_color,acolor,mode&0X1F);
//								}
//								color=piclib_alpha_blend(tmp_color,color,alphabend/8);
//							}else tmp_color=piclib_alpha_blend(acolor,color,alphabend/8);
// 							pic_phy.draw_point(x+tx,y+ty,color);
//							tx++;
//						}
//						bmpbuf+=rowadd;
//						tx=0;
//						ty--;
//					}
//
//				}
//				if(br!=readlen||res)break;
//			}
//		}
//		f_close(f_bmp);
//	}else res=PIC_SIZE_ERR;
//#if BMP_USE_MALLOC == 1
//	pic_memfree(databuf);
//	pic_memfree(f_bmp);
//#endif
//	return res;
//}

//uint8_t bmp_encode(uint8_t *filename, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t mode)
//{
//	FIL* f_bmp;
//	uint16_t bmpheadsize;
// 	BITMAPINFO hbmp;
//	uint8_t res=0;
//	uint16_t tx,ty;
//	uint16_t *databuf;
//	uint16_t pixcnt;
//	uint16_t bi4width;
//	if(width==0||height==0)return PIC_WINDOW_ERR;
//	if((x+width-1)>lcddev.width)return PIC_WINDOW_ERR;
//	if((y+height-1)>lcddev.height)return PIC_WINDOW_ERR;
//
//#if BMP_USE_MALLOC == 1
//	databuf=(uint16_t*)pic_memalloc(1024);
//	if(databuf==NULL)return PIC_MEM_ERR;
//	f_bmp=(FIL *)pic_memalloc(sizeof(FIL));
//	if(f_bmp==NULL)
//	{
//		pic_memfree(databuf);
//		return PIC_MEM_ERR;
//	}
//#else
//	databuf=(uint16_t *)bmpreadbuf;
//	f_bmp = &f_bfile;
//#endif
//	bmpheadsize=sizeof(hbmp);
//	mymemset((uint8_t *)&hbmp, 0, sizeof(hbmp));
//	hbmp.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
//	hbmp.bmiHeader.biWidth=width;
//	hbmp.bmiHeader.biHeight=height;
//	hbmp.bmiHeader.biPlanes=1;
//	hbmp.bmiHeader.biBitCount=16;
//	hbmp.bmiHeader.biCompression=BI_BITFIELDS;
// 	hbmp.bmiHeader.biSizeImage=hbmp.bmiHeader.biHeight*hbmp.bmiHeader.biWidth*hbmp.bmiHeader.biBitCount/8;
//
//	hbmp.bmfHeader.bfType=((uint16_t)'M'<<8)+'B';
//	hbmp.bmfHeader.bfSize=bmpheadsize+hbmp.bmiHeader.biSizeImage;
//   	hbmp.bmfHeader.bfOffBits=bmpheadsize;
//
//	hbmp.RGB_MASK[0]=0X00F800;
//	hbmp.RGB_MASK[1]=0X0007E0;
//	hbmp.RGB_MASK[2]=0X00001F;
//
//	if(mode==1)res=f_open(f_bmp,(const TCHAR*)filename,FA_READ|FA_WRITE);
// 	if(mode==0||res==0x04)res=f_open(f_bmp,(const TCHAR*)filename,FA_WRITE|FA_CREATE_NEW);
// 	if((hbmp.bmiHeader.biWidth*2)%4)
//	{
//		bi4width=((hbmp.bmiHeader.biWidth*2)/4+1)*4;
//	}else bi4width=hbmp.bmiHeader.biWidth*2;
// 	if(res==FR_OK)
//	{
//		res=f_write(f_bmp,(uint8_t*)&hbmp,bmpheadsize,&bw);
//		for(ty=y+height-1;hbmp.bmiHeader.biHeight;ty--)
//		{
//			pixcnt=0;
// 			for(tx=x;pixcnt!=(bi4width/2);)
//			{
//				if(pixcnt<hbmp.bmiHeader.biWidth) databuf[pixcnt] = LCD_ReadPoint(tx,ty);
//				else databuf[pixcnt] = 0Xffff;
//				pixcnt ++;
//				tx ++;
//			}
//			hbmp.bmiHeader.biHeight --;
//			res = f_write(f_bmp, (uint8_t *)databuf, bi4width, &bw);
//		}
//		f_close(f_bmp);
//	}
//#if BMP_USE_MALLOC == 1
//	pic_memfree(databuf);
//	pic_memfree(f_bmp);
//#endif
//	return res;
//}
