#include "y.h"

/* Create a new ybin. */
ybin_t *ybin_new() {
	ybin_t *bin = malloc0(sizeof(ybin_t));
	return (bin);
}
/* Create a new ybin and set its content (data is copied). */
ybin_t *ybin_create(void *data, size_t bytesize) {
	ybin_t *bin = malloc0(sizeof(ybin_t));
	if (!bin || !data || !bytesize)
		return (bin);
	bin->data = malloc0(bytesize);
	if (!bin->data) {
		free0(bin);
		return (NULL);
	}
	memcpy(bin->data, data, bytesize);
	bin->bytesize = bin->buffer_size = bytesize;
	return (bin);
}
/* Create a new ybin and set its content in a larger buffer. */
ybin_t *ybin_create_bufferized(void *data, size_t bytesize) {
	ybin_t *bin = malloc0(sizeof(ybin_t));
	if (!bin || !data || !bytesize)
		return (bin);
	size_t buffer_size = NEXT_POW2(bytesize);
	bin->data = malloc0(buffer_size);
	if (!bin->data) {
		free0(bin);
		return (NULL);
	}
	memcpy(bin->data, data, bytesize);
	bin->bytesize = bytesize;
	bin->buffer_size = buffer_size;
	return (bin);
}
/* Initialize a ybin. The given data is copied. */
ystatus_t ybin_init(ybin_t *bin, void *data, size_t bytesize) {
	if (!bin)
		return (YEUNDEF);
	void *new_data = malloc0(bytesize);
	if (!new_data)
		return (YENOMEM);
	memcpy(new_data, data, bytesize);
	free0(bin->data);
	bin->data = new_data;
	bin->bytesize = bin->buffer_size = bytesize;
	return (YENOERR);
}
/* Initialize a ybin. The given data is copied in a larger buffer. */
ystatus_t ybin_init_bufferized(ybin_t *bin, void *data, size_t bytesize) {
	if (!bin)
		return (YEUNDEF);
	size_t buffer_size = NEXT_POW2(bytesize);
	void *new_data = malloc0(buffer_size);
	if (!new_data)
		return (YENOMEM);
	memcpy(new_data, data, bytesize);
	free0(bin->data);
	bin->data = new_data;
	bin->bytesize = bytesize;
	bin->buffer_size = buffer_size;
	return (YENOERR);
}
/* Clone a ybin and its content. */
ybin_t *ybin_clone(ybin_t *bin) {
	if (!bin)
		return (NULL);
	ybin_t *new_bin = malloc0(sizeof(ybin_t));
	if (!new_bin || !bin->data || !bin->bytesize || !bin->buffer_size)
		return (new_bin);
	new_bin->data = malloc0(bin->buffer_size);
	if (!new_bin->data) {
		free0(new_bin);
		return (NULL);
	}
	memcpy(new_bin->data, bin->data, bin->bytesize);
	new_bin->bytesize = bin->bytesize;
	new_bin->buffer_size = bin->buffer_size;
	return (new_bin);
}
/* Delete a ybin_t structure but not its enclosed data. */
void ybin_free(ybin_t *bin) {
	if (!bin)
		return;
	free0(bin);
}
/* Delete a ybin_t and its enclosed data. */
void ybin_delete(ybin_t *bin) {
	if (!bin)
		return;
	free0(bin->data);
	free0(bin);
}
/* Set a bin_t data pointer (data is not copied). */
void ybin_set(ybin_t *bin, void *data, size_t bytesize) {
	if (!bin)
		return;
	free0(bin->data);
	bin->data = data;
	bin->bytesize = bin->buffer_size = bytesize;
}
/* Reset a ybin_t. */
void ybin_reset(ybin_t *bin) {
	if (!bin)
		return;
	free0(bin->data);
	bin->bytesize = 0;
}
/* Add data at the end of a ybin_t. */
ystatus_t ybin_append(ybin_t *bin, void *data, size_t bytesize) {
	if (!bin)
		return (YEUNDEF);
	if (!data || !bytesize)
		return (YENOERR);
	if (!bin->data) {
		size_t buffer_size = NEXT_POW2(bytesize);
		bin->data = malloc0(bytesize);
		if (!bin->data)
			return (YENOMEM);
		memcpy(bin->data, data, bytesize);
		bin->bytesize = bytesize;
		bin->buffer_size = buffer_size;
		return (YENOERR);
	}
	if (bin->buffer_size > (bin->bytesize + bytesize)) {
		void *pt = (void*)((size_t)bin->data + bin->bytesize);
		memcpy(pt, data, bytesize);
		bin->bytesize += bytesize;
		return (YENOERR);
	}
	size_t buffer_size = NEXT_POW2(bin->bytesize + bytesize);
	void *new_data = malloc0(buffer_size);
	if (!new_data)
		return (YENOMEM);
	memcpy(new_data, bin->data, bin->bytesize);
	void *pt = (void*)((size_t)new_data + bin->bytesize);
	memcpy(pt, data, bytesize);
	free0(bin->data);
	bin->data = new_data;
	bin->bytesize += bytesize;
	bin->buffer_size = buffer_size;
	return (YENOERR);
}
/* Add data at the beginning of a ybin_t. */
ystatus_t ybin_prepend(ybin_t *bin, void *data, size_t bytesize) {
	if (!bin)
		return (YEUNDEF);
	if (!data || !bytesize)
		return (YENOERR);
	if (!bin->data) {
		size_t buffer_size = NEXT_POW2(bytesize);
		bin->data = malloc0(bytesize);
		if (!bin->data)
			return (YENOMEM);
		memcpy(bin->data, data, bytesize);
		bin->bytesize = bytesize;
		bin->buffer_size = buffer_size;
		return (YENOERR);
	}
	if (bin->buffer_size > (bin->bytesize + bytesize)) {
		void *pt = (void*)((size_t)bin->data + bytesize);
		bcopy(bin->data, pt, bin->bytesize);
		memcpy(bin->data, data, bytesize);
		bin->bytesize += bytesize;
		return (YENOERR);
	}
	size_t buffer_size = NEXT_POW2(bin->bytesize + bytesize);
	void *new_data = malloc0(buffer_size);
	if (!new_data)
		return (YENOMEM);
	memcpy(new_data, data, bytesize);
	void *pt = (void*)((size_t)new_data + bytesize);
	memcpy(pt, bin->data, bin->bytesize);
	free0(bin->data);
	bin->data = new_data;
	bin->bytesize += bytesize;
	bin->buffer_size = buffer_size;
	return (YENOERR);
}

