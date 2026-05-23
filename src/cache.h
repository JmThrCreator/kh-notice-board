#pragma once

#include "util.h"
#include "platform/platform.h"

#include "fitz/document.h"
#include "fitz/write-pixmap.h"
#include "fitz/util.h"

#define THUMBNAIL_FOLDER_NAME "thumbnails"
#define THREAD_ARENA_SIZE 32*1024
#define THUMBNAIL_JPEG_QUALITY 90

static const char *ALLOWED_EXTS[] = {"pdf", "png", "jpg", "jpeg"};
#define ALLOWED_EXTS_COUNT (sizeof(ALLOWED_EXTS) / sizeof(ALLOWED_EXTS[0]))

typedef struct CreateFullRenderTask {
	arena_t *arena;
	path_t *file_path;
	path_t *out_dir;
} create_full_render_task_t;

typedef struct CreateThumbnailTask {
	path_t *file_path;
	path_t *out_path;
} create_thumbnail_task_t;

typedef struct PdfInfo {
	fz_context *ctx;
	fz_document *doc;
	int page_count;
} pdf_info_t;

// FILE

err_t copy_file(path_t *src_path, path_t *dst_path) {
	FILE *src_file = fopen(src_path->full_path, "rb");
	if (!src_file) return ERR;

	FILE *dst_file = fopen(dst_path->full_path, "wb");
	if (dst_file == NULL) {
		fclose(src_file);
		return ERR;
	}

	char buf[8192];
	size_t bytes_read;
	while ((bytes_read = fread(buf, 1, sizeof(buf), src_file)) > 0) {
		if (fwrite(buf, 1, bytes_read, dst_file) != bytes_read) {
			fclose(src_file);
			fclose(dst_file);
			return ERR;
		}
	}
	if (ferror(src_file)) {
		fclose(src_file);
		fclose(dst_file);
		return ERR;
	}

	fclose(src_file);
	fclose(dst_file);
	return OK;
}

err_t png_to_jpeg(path_t *src_path, path_t *dst_path) {
	fz_context *ctx = fz_new_context(NULL, NULL, FZ_STORE_UNLIMITED);
	if (!ctx) return false;

	fz_image *image = NULL;
	fz_pixmap *pixmap = NULL;

	fz_try(ctx) {
		fz_open_file(ctx, src_path->full_path);
		image = fz_new_image_from_file(ctx, src_path->full_path);
        	pixmap = fz_get_pixmap_from_image(ctx, image, NULL, NULL, NULL, 0);

        	fz_save_pixmap_as_jpeg(ctx, pixmap, dst_path->full_path, THUMBNAIL_JPEG_QUALITY);
	}
	fz_always(ctx) {
        	if (pixmap) fz_drop_pixmap(ctx, pixmap);
		if (image) fz_drop_image(ctx, image);
        	fz_drop_context(ctx);
	}
	fz_catch(ctx) {
		return ERR;
	}
	return OK;
}

err_t jpeg_to_png(path_t *src_path, path_t *dst_path) {
	fz_context *ctx = fz_new_context(NULL, NULL, FZ_STORE_UNLIMITED);
	if (!ctx) return ERR;

	fz_image *image = NULL;
	fz_pixmap *pixmap = NULL;

	fz_try(ctx) {
		fz_open_file(ctx, src_path->full_path);
		image = fz_new_image_from_file(ctx, src_path->full_path);
        	pixmap = fz_get_pixmap_from_image(ctx, image, NULL, NULL, NULL, 0);

        	fz_save_pixmap_as_png(ctx, pixmap, dst_path->full_path);
    	}
    	fz_always(ctx) {
        	if (pixmap) fz_drop_pixmap(ctx, pixmap);
		if (image) fz_drop_image(ctx, image);
        	fz_drop_context(ctx);
	}
	fz_catch(ctx) {
		return ERR;
	}
    return OK;
}

// PDF

static void ignore_mupdf_msg(void *user, const char *message) { (void)user; (void)message; } // stop mupdf warnings

err_t pdf_info_init(pdf_info_t *pdf_info, path_t *pdf_path) {
	fz_context *ctx = fz_new_context(NULL, NULL, FZ_STORE_UNLIMITED);
	if (!ctx) return ERR;
	fz_set_warning_callback(ctx, ignore_mupdf_msg, NULL);
	fz_set_error_callback(ctx, ignore_mupdf_msg, NULL);

	fz_document *doc;
	int page_count = 1;
	fz_try(ctx) {
		fz_register_document_handlers(ctx);
		doc = fz_open_document(ctx, pdf_path->full_path);
		page_count = fz_count_pages(ctx, doc);
	}
	fz_catch(ctx) {
		fz_drop_context(ctx);
		return ERR;
	}

	pdf_info->ctx = ctx;
	pdf_info->doc = doc;
	pdf_info->page_count = page_count;
	return OK;
}

void pdf_info_free(pdf_info_t *pdf_info) {
	fz_drop_document(pdf_info->ctx, pdf_info->doc);
	fz_drop_context(pdf_info->ctx);
}

err_t convert_pdf_thumbnail(pdf_info_t *pdf_info, path_t *dst_path) {
	fz_pixmap *pixmap;
	fz_try(pdf_info->ctx) {
		fz_matrix matrix = fz_identity;
		pixmap = fz_new_pixmap_from_page_number(pdf_info->ctx, pdf_info->doc, 0, matrix, fz_device_rgb(pdf_info->ctx), 0);
		fz_save_pixmap_as_jpeg(pdf_info->ctx, pixmap, dst_path->full_path, THUMBNAIL_JPEG_QUALITY);
		fz_drop_pixmap(pdf_info->ctx, pixmap);
		pixmap = NULL;
	}
	fz_catch(pdf_info->ctx) {
		if (pixmap) fz_drop_pixmap(pdf_info->ctx, pixmap);
		return ERR;
	}
	return OK;
}

err_t convert_pdf_pages(pdf_info_t *pdf_info, path_t *out_dir, arena_t *scratch) {
	fz_pixmap *pixmap;
	float dpi = 300.0f;
	float scale = dpi / 72.0f;

	fz_try(pdf_info->ctx) {
		fz_matrix matrix = fz_scale(scale, scale);
		for (int i = 0; i < pdf_info->page_count; i++) {
			char file_name[MAX_PATH_LEN];
			if (snprintf(file_name, sizeof(file_name), "page-%i.png", i) >= sizeof(file_name)) return false;
			path_t page_path;
			if (path_init(&page_path, scratch, "", out_dir->full_path, file_name) == ERR) return false;

			pixmap = fz_new_pixmap_from_page_number(pdf_info->ctx, pdf_info->doc, i, matrix, fz_device_rgb(pdf_info->ctx), 0);
			fz_save_pixmap_as_png(pdf_info->ctx, pixmap, page_path.full_path);
			fz_drop_pixmap(pdf_info->ctx, pixmap);
			pixmap = NULL;
		}
	}
	fz_catch(pdf_info->ctx) {
		if (pixmap) fz_drop_pixmap(pdf_info->ctx, pixmap);
		return ERR;
	}
	return OK;
}

// THREAD FUNCS

void *create_thumbnail(void *arg) {
	create_thumbnail_task_t *task = (create_thumbnail_task_t *)arg;
	path_t *file_path = task->file_path;
	path_t *out_path = task->out_path;

	if (strncmp(file_path->ext, "pdf", MAX_PATH_LEN) == 0) {
		pdf_info_t pdf_info = {0};
		if (pdf_info_init(&pdf_info, file_path) == ERR) return NULL;
		if (convert_pdf_thumbnail(&pdf_info, out_path) == ERR) return NULL;
		pdf_info_free(&pdf_info);
	}
	else if (strncmp(file_path->ext, "png", MAX_PATH_LEN) == 0) {
		png_to_jpeg(file_path, out_path);
	}
	else if (strstr("jpg, jpeg", file_path->ext)) {
		copy_file(file_path, out_path);
	}
	return NULL;
}

void *create_full_render(void *arg) {
	create_full_render_task_t *task = (create_full_render_task_t *)arg;
	arena_t *arena = task->arena;
	path_t *file_path = task->file_path;
	path_t *out_dir = task->out_dir;

	if (file_path->ext == NULL || strncmp(file_path->ext, "pdf", MAX_PATH_LEN) == 0) {
		pdf_info_t pdf_info = {0};
		if (pdf_info_init(&pdf_info, file_path) == ERR) return NULL;
		if (convert_pdf_pages(&pdf_info, out_dir, arena) == ERR) return NULL;
		pdf_info_free(&pdf_info);
	}
	else {
		path_t out_path;
		if (path_init(&out_path, arena, "", out_dir->full_path, "0.png") == ERR) return NULL;
		if (strstr("jpg,jpeg", file_path->ext)) {
			if (jpeg_to_png(file_path, &out_path) == ERR) return NULL;
		} else {
			if (copy_file(file_path, &out_path) == ERR) return NULL;
		}
	}

	return NULL;
}

err_t cache_copy_structure(arena_t *scratch, path_t *input_dir, path_t *cache_path) {
	pathlist_t sub_folders;
	pathlist_init(&sub_folders, scratch, input_dir->full_path, PATHLIST_RECURSIVE);
	pathlist_filter_by_ext(&sub_folders, ALLOWED_EXTS, ALLOWED_EXTS_COUNT);

	for (int j = 0; j < sub_folders.count; j++) {
		path_t sub_folder;
		try(path_init(
			&sub_folder, scratch, "", cache_path->full_path,
			sub_folders.items[j]->relative_prefix, sub_folders.items[j]->name));
		try(create_dir(&sub_folder));
	}
	return OK;
}

// is root = is this the cache folder? or is it a sub folder?
err_t cache_render_thumbnails(arena_t *scratch, path_t *input_dir, path_t *cache_path, bool is_root) {
	pathlist_t real_sub_folders; // exclude doc folders
	try(pathlist_init(&real_sub_folders, scratch, input_dir->full_path, PATHLIST_RECURSIVE | PATHLIST_DIRS_ONLY));

	// create thumbnail at root when ran at the folder level
	if (!is_root) {
		path_t top_thumbnail_folder;
		try(path_init(&top_thumbnail_folder, scratch, "", cache_path->full_path, THUMBNAIL_FOLDER_NAME));
		try(create_dir(&top_thumbnail_folder));
	}	

	for (int j = 0; j < real_sub_folders.count; j++) {
		path_t thumbnail_folder;
		try(path_init(&thumbnail_folder, scratch, "", cache_path->full_path,
			real_sub_folders.items[j]->relative_prefix, real_sub_folders.items[j]->name, THUMBNAIL_FOLDER_NAME));
		try(create_dir(&thumbnail_folder));
	}

	pathlist_t files;
	try(pathlist_init(&files, scratch, input_dir->full_path, PATHLIST_RECURSIVE | PATHLIST_FILES_ONLY));
	pathlist_filter_by_ext(&files, ALLOWED_EXTS, ALLOWED_EXTS_COUNT);
	
	kh_thread_t *threads[MAX_THREADS];
	int threads_running = 0;
	for (int j = 0; j < files.count; j++) {
		path_t *file_path = files.items[j];

		char thumbnail_out_name[MAX_PATH_LEN];
		if (snprintf(thumbnail_out_name, sizeof(thumbnail_out_name), "%s.jpeg", file_path->name) >= sizeof(thumbnail_out_name)) continue;
	
		path_t *thumbnail_out = arena_push_struct(scratch, path_t);
		try(path_init(thumbnail_out, scratch, "", cache_path->full_path,
			file_path->relative_prefix, THUMBNAIL_FOLDER_NAME, thumbnail_out_name));
		
		create_thumbnail_task_t *task = arena_push_struct(scratch, create_thumbnail_task_t);
		task->file_path = file_path;
		task->out_path = thumbnail_out;

		try(thread_create(scratch, &threads[threads_running], create_thumbnail, task));
		threads_running++;

		if (threads_running == MAX_THREADS) {
			for (int k = 0; k < threads_running; k++) {
				try(thread_join(threads[k]));
			}
			threads_running = 0;
		}
	}
	for (int k = 0; k < threads_running; k++) {
		try(thread_join(threads[k]));
	}
	return OK;
}

err_t cache_create_full_render(arena_t *scratch, path_t *input_dir, path_t *cache_path) {
	pathlist_t files;
	try(pathlist_init(&files, scratch, input_dir->full_path, PATHLIST_RECURSIVE | PATHLIST_FILES_ONLY));
	pathlist_filter_by_ext(&files, ALLOWED_EXTS, ALLOWED_EXTS_COUNT);

	kh_thread_t *threads[MAX_THREADS];
	int threads_running = 0;

	// each thread has its own arena space, which it reuses for every task
	arena_t thread_arenas[MAX_THREADS];
	uint8_t *block = arena_push_array(scratch, MAX_THREADS * THREAD_ARENA_SIZE, uint8_t);
	for (int i = 0; i < MAX_THREADS; i++) {
		thread_arenas[i].base = block + i * THREAD_ARENA_SIZE;
    		thread_arenas[i].capacity = THREAD_ARENA_SIZE;
    		thread_arenas[i].offset = 0;
	}
	
	threads_running = 0;
	for (int j = 0; j < files.count; j++) {
		path_t *file_path = files.items[j];
	
		path_t *out_dir = arena_push_struct(scratch, path_t);
		path_init(out_dir, scratch, "", cache_path->full_path,
			file_path->relative_prefix, file_path->name);
	
		int thread_index = threads_running;
		arena_t *thread_arena = &thread_arenas[thread_index];
		
		create_full_render_task_t *task = arena_push_struct(scratch, create_full_render_task_t);
		task->arena = thread_arena;
		task->file_path = file_path;
		task->out_dir = out_dir;

		try(thread_create(scratch, &threads[threads_running], create_full_render, task));
		threads_running++;

		if (threads_running == MAX_THREADS) {
			for (int k = 0; k < threads_running; k++) {
				try(thread_join(threads[k]));
				thread_arenas[k].offset = 0;
			}
			threads_running = 0;
		}
	}
	for (int k = 0; k < threads_running; k++) {
		try(thread_join(threads[k]));
	}
	return OK;
}
