all:
	@echo "CompileFlags:" >> .clangd
	@echo "  Add: [-std=c99, -Wall, -fopenmp, -I.]" >> .clangd
	@echo "  Compiler: gcc" >> .clangd
	@includes=""; \
	grep -E '^#include "[^"]+\.c"' main.c | sed 's/.*"\([^"]*\)".*/\1/' | while read file; do \
		if [ -n "$$includes" ]; then \
			echo "---" >> .clangd; \
			echo "If:" >> .clangd; \
			echo "  PathMatch: $$(echo $$file | sed 's/\./\\\\./g')" >> .clangd; \
			echo "CompileFlags:" >> .clangd; \
			echo "  Add: [$$includes]" >> .clangd; \
		fi; \
		includes="$$includes$${includes:+, }-include, $$file"; \
	done
	@echo "Generated .clangd from main.c includes"
	gcc -std=c99 -O3 -ffast-math -fopenmp -march=native -Wall -Werror main.c -o craytracer -lm -lSDL2

