/**
 * FreeRDS: FreeRDP Remote Desktop Services (RDS)
 *
 * Copyright 2005-2012 Jay Sorg
 * Copyright 2013-2014 Marc-Andre Moreau <marcandre.moreau@gmail.com>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "rdp.h"
#include "rdpDraw.h"

#define LDEBUG 0

#define LOG_LEVEL 1
#define LLOG(_level, _args) \
		do { if (_level < LOG_LEVEL) { ErrorF _args ; } } while (0)
#define LLOGLN(_level, _args) \
		do { if (_level < LOG_LEVEL) { ErrorF _args ; ErrorF("\n"); } } while (0)

extern DevPrivateKeyRec g_rdpGCIndex;
extern DevPrivateKeyRec g_rdpPixmapIndex;
extern rdpPixmapRec g_screenPriv;

extern GCOps g_rdpGCOps;

void rdpPushPixelsOrg(GCPtr pGC, PixmapPtr pBitMap, DrawablePtr pDst, int w, int h, int x, int y)
{
	rdpGCPtr priv;
	GCFuncs *oldFuncs;

	GC_OP_PROLOGUE(pGC);
	pGC->ops->PushPixels(pGC, pBitMap, pDst, w, h, x, y);
	GC_OP_EPILOGUE(pGC);
}

void rdpPushPixels(GCPtr pGC, PixmapPtr pBitMap, DrawablePtr pDst, int w, int h, int x, int y)
{
	RegionRec clip_reg;
	RegionRec box_reg;
	int num_clips;
	int cd;
	int j;
	int post_process;
	BoxRec box;
	WindowPtr pDstWnd;
	PixmapPtr pDstPixmap;
	rdpPixmapRec *pDstPriv;

	LLOGLN(10, ("rdpPushPixels:"));

	/* do original call */
	rdpPushPixelsOrg(pGC, pBitMap, pDst, w, h, x, y);

	post_process = 0;

	if (pDst->type == DRAWABLE_PIXMAP)
	{
		pDstPixmap = (PixmapPtr) pDst;
		pDstPriv = GETPIXPRIV(pDstPixmap);
	}
	else
	{
		if (pDst->type == DRAWABLE_WINDOW)
		{
			pDstWnd = (WindowPtr) pDst;

			if (pDstWnd->viewable)
			{
				post_process = 1;
			}
		}
	}

	if (!post_process)
		return;

	memset(&box, 0, sizeof(box));
	RegionInit(&clip_reg, NullBox, 0);
	cd = rdp_get_clip(&clip_reg, pDst, pGC);

	if (cd == 1)
	{
		rdp_send_area_update(pDst->x + x, pDst->y + y, w, h);
	}
	else if (cd == 2)
	{
		box.x1 = pDst->x + x;
		box.y1 = pDst->y + y;
		box.x2 = box.x1 + w;
		box.y2 = box.y1 + h;
		RegionInit(&box_reg, &box, 0);
		RegionIntersect(&clip_reg, &clip_reg, &box_reg);
		num_clips = REGION_NUM_RECTS(&clip_reg);

		if (num_clips > 0)
		{
			for (j = num_clips - 1; j >= 0; j--)
			{
				box = REGION_RECTS(&clip_reg)[j];
				rdp_send_area_update(box.x1, box.y1, box.x2 - box.x1, box.y2 - box.y1);
			}
		}

		RegionUninit(&box_reg);
	}

	RegionUninit(&clip_reg);
}
