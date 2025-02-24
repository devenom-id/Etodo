# Etodo rewritten - started on 24/2

Programa de lista de tareas con sincronización e interfaz responsive.

# REQUISITOS
1. Lenguaje C
2. Interfaz con ncurses
3. Mostrar las tareas añadidas a la lista
4. Guardar las tareas y datos asociados en un archivo json
5. Ofrecer funciones para añadir, borrar, editar y reordenar las tareas
6. Redimensionar interfaz automáticamente al cambiar el tamaño de la ventana
7. Ofrecer la posibilidad de sincronizar las tareas mediante internet

---

# COMPONENTES

1. sobre json
2. sobre operaciones de tareas
3. sobre interfaz y menúes
4. sobre redimensión
5. sobre sincronización
6. sobre debugging

## JSON
1. Cargar archivo
2. Volcar archivo

:: En vez de una estructura propia puedo usar las propias estructuras de json.h

## Operaciones de tareas
1. Añadir tarea
2. Borrar tarea
3. Editar tarea
4. Reordenar tarea hacia arriba
5. Reordenar tarea hacia abajo

## Sobre interfaz y menúes
1. 

## Sobre redimensión
1. Handler para señal de redimensión
2. Función de redimensión

## Sobre sincronización

## Sobre debugging
1. Archivo DEBUG
2. Función de registro de variable

---

# ESTRUCTURA
- Inicio de ncurses y definiciones básicas como colores
- Carga de tareas desde json
- Listar tareas
- Habilitar interacción con la lista de tareas : Añadir : Borrar : Editar : Reordenar : Marcar


# ESTRUCTURA DE JSON
string tarea: [int estado, string hora]