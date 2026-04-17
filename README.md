# Codexion

*Este proyecto ha sido creado como parte del currículo de 42.*

## 📌 Descripción

**Codexion** es una simulación de concurrencia en C que modela múltiples programadores compitiendo por recursos limitados (dongles USB) para poder compilar su código.

Cada programador está representado por un hilo (`thread`) y debe gestionar correctamente el acceso a los recursos compartidos mediante mecanismos de sincronización como **mutexes** y **variables de condición**.

El objetivo principal del proyecto es diseñar un sistema que:

* Evite condiciones de carrera.
* Prevenga interbloqueos (*deadlocks*).
* Garantice equidad en el acceso a recursos.
* Controle correctamente el tiempo para evitar el "agotamiento" (*burnout*) de los programadores.

Este proyecto pone el foco en problemas clásicos de concurrencia, similares al conocido problema de los filósofos comensales, pero con restricciones adicionales como cooldown de recursos y planificación avanzada.

---

## ⚙️ Características principales

* Simulación basada en **threads POSIX (`pthread`)**
* Gestión de recursos compartidos mediante **mutexes**
* Uso de **variables de condición** para coordinación eficiente
* Implementación de planificación:

  * **FIFO (First In, First Out)**
  * **EDF (Earliest Deadline First)**
* Sistema de logging sincronizado
* Detección precisa de condiciones de fallo (*burnout*)
* Control de tiempo en milisegundos

---

## 🧠 Conceptos clave

Este proyecto permite trabajar y comprender en profundidad:

* Programación concurrente
* Sincronización de hilos
* Condiciones de carrera (*race conditions*)
* Interbloqueos (*deadlocks*)
* Inanición (*starvation*)
* Planificación de procesos
* Gestión de recursos limitados

---

## 🚀 Ejecución

```bash
./codexion number_of_coders time_to_burnout time_to_compile time_to_debug time_to_refactor number_of_compiles_required dongle_cooldown scheduler
```

### Ejemplo:

```bash
./codexion 5 800 200 200 200 5 50 edf
```

---

## 📊 Output

El programa muestra en tiempo real el estado de cada programador:

```
timestamp X has taken a dongle
timestamp X is compiling
timestamp X is debugging
timestamp X is refactoring
timestamp X burned out
```

---

## ⚠️ Requisitos

* Sin variables globales
* Sin fugas de memoria
* Cumplimiento estricto de la norma de 42
* Compilación con:

  ```
  -Wall -Wextra -Werror -pthread
  ```

---

## 🧩 Retos técnicos

* Evitar deadlocks sin sacrificar rendimiento
* Implementar un sistema justo de asignación de recursos
* Manejar correctamente el cooldown de los recursos
* Sincronizar correctamente el logging entre hilos
* Detectar el burnout con precisión temporal

---

## 📚 Inspiración

Este proyecto está inspirado en problemas clásicos de concurrencia como:

* Dining Philosophers Problem

Pero introduce complejidades adicionales como:

* Planificación dinámica (EDF)
* Restricciones temporales estrictas
* Recursos con cooldown

---

## 📝 Notas

El enfoque del proyecto no es solo que funcione, sino que sea **explicable, robusto y libre de errores de concurrencia**, algo crítico en sistemas reales.

---

## 🤝 Autor

Kevin
42 Student

