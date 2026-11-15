# Documentación de Simurgh

Para desarrolladores y agentes de IA. Resumen de decisiones en [`../AGENTS.md`](../AGENTS.md).

| Documento | Contenido |
|---|---|
| [GENEALOGIA.md](GENEALOGIA.md) | De DIV Games Studio a Fenix, Bennu y Simurgh; origen exacto del código |
| [ARQUITECTURA.md](ARQUITECTURA.md) | Compilador, intérprete, capa glue, módulos del código heredado y tamaños |
| [COLOR-Y-FORMATOS.md](COLOR-Y-FORMATOS.md) | 16 bits, transparencia, compatibilidad de FPG y MAP, temas abiertos |
| [PLATAFORMAS.md](PLATAFORMAS.md) | Windows, Linux, macOS, OLPC XO-1 y DOS: requisitos y particularidades |
| [COMPILACION.md](COMPILACION.md) | Compiladores, entornos, SDL y la compilación verificada de Fenix 0.92a para el XO |
| [PROBLEMAS.md](PROBLEMAS.md) | Lista de problemas conocidos del código heredado y del plan |
| [IDE-FENXEDIT2.md](IDE-FENXEDIT2.md) | FenXEdit2, el IDE escrito en Fenix, parte del proyecto |
| [MIGRACION-FENIX.md](MIGRACION-FENIX.md) | Cambios del lenguaje respecto a Fenix (se apunta cada uno al decidirlo) |
| [FORMATOS-IMAGEN.md](FORMATOS-IMAGEN.md) | Formatos de imagen y recursos, `LOAD_IMAGE` por cabecera y alias del compilador |
| [HOJA-DE-RUTA.md](HOJA-DE-RUTA.md) | Fases, pruebas y temas pendientes antes de la primera versión |
| [../tools/upstream-diff.sh](../tools/upstream-diff.sh) | Compara `src/` con `upstream/` (lista, resumen, diff) y comprueba los avisos de cambio exigidos por la GPL |
| [../upstream/CAMBIOS.md](../upstream/CAMBIOS.md) | Fenix 0.92a corregido (versión de referencia), con su diff contra el original |
| [../tools/zig-probe.sh](../tools/zig-probe.sh) | Compila `fxc` con Zig (sin SDL) y comprueba que el `.dcb` es reproducible; ver `COMPILACION.md` |
| [../tools/zig-probe-fxi.sh](../tools/zig-probe-fxi.sh) | Compila `fxi` de 32 bits para Windows con Zig y SDL 1.2 y deja un directorio ejecutable |
| [../tools/fxc-compare.sh](../tools/fxc-compare.sh) | Pruebas de referencia del compilador: `src/` frente a `upstream/fenix-0.92a-fixed` sobre `tests/fxc/` |
| [../tools/fxc-reentrant.sh](../tools/fxc-reentrant.sh) | Prueba de reentrancia de `fxc` como biblioteca: serie con errores y varios hilos |
| [../tools/ide-compile.sh](../tools/ide-compile.sh) | Compila el IDE (`ide/FenXEdit2`) |
| [../tools/image-test.sh](../tools/image-test.sh) | Prueba de `LOAD_IMAGE`, alias, `SAVE_MAP` y `SAVE_FPG` (ejecuta `fxi` de 32 bits) |
| [fenix-xo-0.92a.patch](fenix-xo-0.92a.patch) | Cambios mínimos para compilar Fenix 0.92a con libpng 1.5 y enlazado actual |
