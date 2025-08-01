#include <poppler.h>
#include <cairo.h>
#include <cairo-pdf.h>
#include <stdio.h>

#include "fs_utils.h"

#define DPI 300

int pdf_to_png(Path *pdf_path, const char *output_dir) {
	char *uri = g_filename_to_uri(pdf_path->path, NULL, NULL);
	if (!uri) { perror("g_filename_to_uri"); return 1; }
	
	PopplerDocument *document = poppler_document_new_from_file(uri, NULL, NULL);
	g_free(uri);
	if (!document) { perror("poppler_document_new_from_file"); return 1; }

	int page_count = poppler_document_get_n_pages(document);

	for (int page_num = 0; page_num < page_count; page_num++) {
		PopplerPage *page = poppler_document_get_page(document, page_num);
		if (!page) { perror("poppler_document_get_page"); continue; }

		double width, height;
		poppler_page_get_size(page, &width, &height);
		
		double scale = DPI / 72.0;
		int scaled_width = (int)(width * scale);
		int scaled_height = (int)(height * scale);

		// surface to render PNG
		cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, scaled_width, scaled_height);
		cairo_t *cr = cairo_create(surface);
		cairo_scale(cr, scale, scale);
		
		cairo_set_source_rgb(cr, 1, 1, 1);
		cairo_paint(cr);
		
		poppler_page_render(page, cr);

		char output_path [PATH_LENGTH];
		snprintf(output_path, sizeof(output_path), "%s/%i.png", output_dir, page_num);
		cairo_surface_write_to_png(surface, output_path);

		// cleanup
		cairo_destroy(cr);
		cairo_surface_destroy(surface);
		g_object_unref(page);
	}
	g_object_unref(document);
	return 0;
}

int test(int argc, char *argv[]) {
	if (argc != 3) {
		fprintf(stderr, "Usage: %s <input.pdf> <output-page-[n].png>\n", argv[0]);
		return 1;
	}
	//pdf_to_png(argv[1], argv[2]);
	return 0;
}
