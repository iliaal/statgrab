$(STATGRAB_SHARED_DEPENDENCIES):
	@echo "Bundled libstatgrab archive missing: $@" >&2
	@echo "PHP's 'make clean' deletes it. Run 'make -C vendor/libstatgrab', then re-run make." >&2
	@exit 1
