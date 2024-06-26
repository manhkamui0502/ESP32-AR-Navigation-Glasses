#include "lv_fs_littlefs.h"
#include <lvgl.h>
#include <LittleFS.h>

struct LittleFile
{
    fs::File file;
};

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void fs_init(void);
static void *fs_open(lv_fs_drv_t *drv, const char *path, lv_fs_mode_t mode);
static lv_fs_res_t fs_close(lv_fs_drv_t *drv, void *file_p);
static lv_fs_res_t fs_read(lv_fs_drv_t *drv, void *file_p, void *buf, uint32_t btr, uint32_t *br);
static lv_fs_res_t fs_write(lv_fs_drv_t *drv, void *file_p, const void *buf, uint32_t btw, uint32_t *bw);
static lv_fs_res_t fs_seek(lv_fs_drv_t *drv, void *file_p, uint32_t pos, lv_fs_whence_t whence);
static lv_fs_res_t fs_tell(lv_fs_drv_t *drv, void *file_p, uint32_t *pos_p);

void lv_fs_littlefs_init(void)
{
    fs_init();
    static lv_fs_drv_t fs_drv;
    lv_fs_drv_init(&fs_drv);

    fs_drv.letter = 'F';
    fs_drv.open_cb = fs_open;
    fs_drv.close_cb = fs_close;
    fs_drv.read_cb = fs_read;
    fs_drv.write_cb = fs_write;
    fs_drv.seek_cb = fs_seek;
    fs_drv.tell_cb = fs_tell;

    fs_drv.dir_close_cb = NULL;
    fs_drv.dir_open_cb = NULL;
    fs_drv.dir_read_cb = NULL;

    lv_fs_drv_register(&fs_drv);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/*Initialize your Storage device and File system.*/
static void fs_init(void) { LittleFS.begin(); }

static void *fs_open(lv_fs_drv_t *drv, const char *path, lv_fs_mode_t mode)
{
    LV_UNUSED(drv);
    const char *flags = "";
    if (mode == LV_FS_MODE_WR)
        flags = FILE_WRITE;
    else if (mode == LV_FS_MODE_RD)
        flags = FILE_READ;
    else if (mode == (LV_FS_MODE_WR | LV_FS_MODE_RD))
        flags = FILE_WRITE;

    fs::File f = LittleFS.open(path, flags);
    if (!f)
    {
        return NULL;
    }
    LittleFile *lf = new LittleFile{f};
    return (void *)lf;
}

static lv_fs_res_t fs_close(lv_fs_drv_t *drv, void *file_p)
{
    LV_UNUSED(drv);
    LittleFile *lf = (LittleFile *)file_p;

    lf->file.close();

    delete lf;
    return LV_FS_RES_OK;
}

static lv_fs_res_t fs_read(lv_fs_drv_t *drv, void *file_p, void *buf, uint32_t btr, uint32_t *br)
{
    LV_UNUSED(drv);
    LittleFile *lf = (LittleFile *)file_p;

    *br = lf->file.read((uint8_t *)buf, btr);

    return (int32_t)(*br) < 0 ? LV_FS_RES_UNKNOWN : LV_FS_RES_OK;
}

static lv_fs_res_t fs_write(lv_fs_drv_t *drv, void *file_p, const void *buf, uint32_t btw, uint32_t *bw)
{
    LV_UNUSED(drv);
    LittleFile *lf = (LittleFile *)file_p;
    *bw = lf->file.write((uint8_t *)buf, btw);

    return (int32_t)(*bw) < 0 ? LV_FS_RES_UNKNOWN : LV_FS_RES_OK;
}

static lv_fs_res_t fs_seek(lv_fs_drv_t *drv, void *file_p, uint32_t pos, lv_fs_whence_t whence)
{
    LV_UNUSED(drv);
    fs::SeekMode mode;
    if (whence == LV_FS_SEEK_SET)
        mode = fs::SeekSet;
    else if (whence == LV_FS_SEEK_CUR)
        mode = fs::SeekCur;
    else if (whence == LV_FS_SEEK_END)
        mode = fs::SeekEnd;

    LittleFile *lf = (LittleFile *)file_p;
    lf->file.seek(pos, mode);

    return LV_FS_RES_OK;
}

static lv_fs_res_t fs_tell(lv_fs_drv_t *drv, void *file_p, uint32_t *pos_p)
{
    LV_UNUSED(drv);
    LittleFile *lf = (LittleFile *)file_p;
    *pos_p = lf->file.position();

    return LV_FS_RES_OK;
}