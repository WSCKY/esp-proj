#include "bmpDec.h"
#include "string.h"
#include "esp_log.h"
#include "esp_heap_caps.h"

#define RGB_2_BYTES(a, b)      (((uint16_t)(a) << 8) | (b))
#define RGB555_2_BYTES(a, b)   (((uint16_t)(a) << 9) | (((uint16_t)(b) & 0xE0) << 1 ) | ((b) & 0x1F))
#define RGB_3_BYTES(a, b, c)   ((((uint16_t)(a) << 8) & 0xF800) | (((uint16_t)(b) << 3) & 0x07E0) | ((c) >> 3))

static const char *TAG = "BMP_DEC";

uint16_t ImgWidth = 0, ImgHeight = 0;

#define PARALLEL_LINES         8

esp_err_t bmp_decode(const char *path, pDrawPrepare_t pDrawPrepare, pFillScreen_t pFillScreen)
{
	esp_err_t ret = ESP_OK;
    uint16_t cnt = 0;
	uint8_t color_byte;
	uint16_t color = 0;
	uint16_t pixel_cnt = 0;

	uint8_t *databuf = NULL;
	uint16_t readlen = 0;
	uint16_t line_cnt = 0;
	uint16_t *line_data = NULL;

	uint16_t data_offset = 0;
	uint8_t biCompression = 0;

	uint8_t extra = 0;
	uint16_t line_bytes = 0;
	ImgArea_t ImgRect = {0, 0, 0, 0};

	databuf = (uint8_t *)calloc(1, sizeof(BITMAPINFO));
	assert(databuf != NULL);

	FILE *f = fopen(path, "r"); // read only.
	if(f == NULL) {
		ESP_LOGE(TAG, "can't open file %s", path);
		ret = ESP_FAIL;
	} else {
		readlen = fread(databuf, sizeof(BITMAPINFO), 1, f);
		if(readlen > 0) {
			BITMAPINFO *pbmp = (BITMAPINFO *)databuf;
			// Copy data.
			data_offset = pbmp->bmfHeader.bfOffBits;
			color_byte = pbmp->bmiHeader.biBitCount >> 3;
			biCompression = pbmp->bmiHeader.biCompression;
			ImgHeight = pbmp->bmiHeader.biHeight;
			ImgWidth = pbmp->bmiHeader.biWidth;
			ESP_LOGI(TAG, "BMP, %dx%d, %dbit", (int)(pbmp->bmiHeader.biWidth), (int)(pbmp->bmiHeader.biHeight), (int)(pbmp->bmiHeader.biBitCount));

			if(color_byte < 2 || color_byte > 4 ) {
				ESP_LOGE(TAG, "unsupport %dbit color", (int)(pbmp->bmiHeader.biBitCount));
				fclose(f);
				ret = ESP_FAIL;
				goto exit;
			}
			free(databuf);
			databuf = NULL;

			if((ImgWidth * color_byte) % 4)
				line_bytes = ((ImgWidth * color_byte) / 4 + 1) * 4;
			else
				line_bytes = ImgWidth * color_byte;
			extra = line_bytes - ImgWidth * color_byte;

			ImgRect.left = 0;
			ImgRect.top = 0;
			ImgRect.right = ImgWidth - 1;
			ImgRect.bottom = ImgHeight - 1;
			if(pDrawPrepare(&ImgRect) != ESP_OK) {
				ESP_LOGE(TAG, "BMP Size unsupport.");
				fclose(f);
				ret = ESP_FAIL;
				goto exit;
			}

			fseek(f, data_offset, SEEK_SET);

			databuf = (uint8_t *)malloc(line_bytes * PARALLEL_LINES);
			line_data = (uint16_t *)heap_caps_malloc(ImgWidth * sizeof(uint16_t) * PARALLEL_LINES, MALLOC_CAP_DMA);
			assert(line_data != NULL);

			while((readlen = fread(databuf, sizeof(uint8_t), line_bytes * PARALLEL_LINES, f)) > 0) {
				cnt = 0;
				while(cnt < readlen) {
					switch(color_byte) {
					case 1:
						//unsupport.
						break;
					case 2:
						if(biCompression == BI_RGB) {
							color = RGB555_2_BYTES(databuf[cnt + 1], databuf[cnt]);
						} else {
							color = RGB_2_BYTES(databuf[cnt + 1], databuf[cnt]);
						}
						break;
					case 3:
						color = RGB_3_BYTES(databuf[cnt + 2], databuf[cnt + 1], databuf[cnt]);
						break;
					case 4:
						color = RGB_3_BYTES(databuf[cnt + 2], databuf[cnt + 1], databuf[cnt]);
						// ... and ignore the databuf[cnt + 3].
						break;
					default:
						//unsupport.
						break;
					}
					cnt += color_byte;
					line_data[pixel_cnt + line_cnt * ImgWidth] = color;
					if( ++pixel_cnt >= ImgWidth) {
						cnt += extra;
						pixel_cnt = 0;
						line_cnt ++;
						if(line_cnt >= PARALLEL_LINES) {
							pFillScreen(line_data, line_cnt * ImgWidth, true);
							line_cnt = 0;
						}
					}
				}
			}
			if(line_cnt > 0)
				pFillScreen(line_data, line_cnt * ImgWidth, true);
			fclose(f);
		} else {
			ESP_LOGE(TAG, "can't read data from %s", path);
			fclose(f);
			ret = ESP_FAIL;
		}
	}

exit:
	if(databuf != NULL) free(databuf);
	heap_caps_free(line_data);
	return ret;
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
