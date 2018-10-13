#include "efi/boot-services.h"
#include "efi/runtime-services.h"
#include "efi/system-table.h"
#include "efi/types.h"
#include "stdfunctions.h"

static efi_handle image_handle;
static efi_system_table *st;

void *malloc(size_t count) {
	void *ptr;
	if (st->BootServices->AllocatePool(EfiLoaderData, count, &ptr) != EFI_SUCCESS) {
		return ptr;
	}
	return NULL;
}

void *calloc(size_t size, size_t count) {
	void *ptr = malloc(size * count);
	if (ptr) {
		memset(ptr, 0, size * count);
	}
	return ptr;
}

void free(void *ptr) {
	st->BootServices->FreePool(ptr);
}

size_t get_free_memory_size(void) {
	return 0; // TODO
}

NO_RETURN void reboot(void) {
	puts("Press any key...\r\n");
	st->ConIn->Reset(st->ConIn, false);
	efi_event events[] = { st->ConIn->WaitForKey };
	st->BootServices->WaitForEvent(1, events, NULL);
	st->BootServices->Exit(image_handle, EFI_SUCCESS, 0, NULL);
	while (true);
}

void puts(const char *s) {
	size_t len = strlen(s);
	if (len < 256) {
		char16_t buffer[256];
		for (size_t i = 0; i <= len; i++) {
			buffer[i] = (char16_t) s[i];
		}
		st->ConOut->OutputString(st->ConOut, buffer);
	} else {
		char16_t *buffer = malloc((len + 1) * sizeof(wchar_t));
		for (size_t i = 0; i <= len; i++) {
			buffer[i] = (char16_t) s[i];
		}
		st->ConOut->OutputString(st->ConOut, buffer);
		free(buffer);
	}
}

efi_status efi_main(efi_handle handle __attribute__((unused)), efi_system_table *st_) {
	image_handle = handle;
	st = st_;
	main();
	reboot();
}
