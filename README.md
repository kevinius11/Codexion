*Este proyecto ha sido creado como parte del currículo de 42 por kcastro-.*

# Codexion

## Descripción

Codexion es una simulación concurrente inspirada en el clásico problema de los *Dining Philosophers*, adaptado a un entorno de compilación distribuida donde múltiples *coders* compiten por recursos limitados llamados *dongles*.

El proyecto modela problemas reales de concurrencia de bajo nivel utilizando `pthreads`, sincronización mediante mutexes y variables de condición, y políticas avanzadas de scheduling como:

- **FIFO (First In First Out)**
- **EDF (Earliest Deadline First)**

El objetivo principal del proyecto es estudiar y resolver problemas clásicos de sistemas concurrentes:

- Deadlocks
- Starvation
- Race conditions
- Resource contention
- Scheduling fairness
- Thread-safe communication

---

# Arquitectura General

Cada *coder* es representado por un hilo (`pthread`) que necesita adquirir dos *dongles* exclusivos para poder compilar.

El sistema está compuesto por:

| Componente | Rol |
|---|---|
| Coders | Workers concurrentes |
| Dongles | Recursos compartidos exclusivos |
| Heap Scheduler | Cola de prioridad para arbitraje |
| Monitor Thread | Supervisión global de burnout |
| Mutexes | Exclusión mutua |
| Condition Variables | Comunicación y espera eficiente |

---

# Modelo de Concurrencia

Codexion implementa una combinación de:

- Dining Philosophers
- Resource Arbitration
- Priority Scheduling
- Monitor Pattern
- Shared Memory Concurrency

---

# Características Técnicas

## Scheduling híbrido

El sistema soporta dos políticas de scheduling:

### FIFO

Prioriza al coder que llegó primero a la cola de espera.

### EDF (Earliest Deadline First)

Prioriza al coder más cercano al burnout utilizando deadlines dinámicos:

```text
deadline = last_compilation + time_to_burnout
```

---

## Priority Queue basada en Min Heap

Cada dongle mantiene una cola de prioridad independiente:

```c
t_heap queue;
```

La prioridad se calcula mediante:

- timestamp
- coder_id (tie-break)

### Complejidades

| Operación | Complejidad |
|---|---|
| Insert | O(log n) |
| Extract Min | O(log n) |
| Peek | O(1) |

---

## Backoff Exponencial

Para reducir contention y retry storms:

```text
200µs → 400µs → 800µs → ...
```

Esto evita sincronización accidental entre hilos y mejora fairness.

---

# Instrucciones

## Compilación

```bash
make
```

---

## Ejecución

```bash
./codexion \
[number_of_coders] \
[time_to_burnout] \
[time_to_compile] \
[time_to_debug] \
[time_to_refactor] \
[number_of_compiles_required] \
[dongle_cooldown] \
[scheduler]
```

---

## Ejemplo FIFO

```bash
./codexion 5 800 200 200 200 5 50 fifo
```

---

## Ejemplo EDF

```bash
./codexion 100 300 50 50 50 1 50 edf
```

---

# Visualización de Datos

## Resaltar compilaciones y contar total

```bash
./codexion 100 300 50 50 50 1 50 edf \
| awk '/is compiling/{c++;gsub(/is compiling/,"\033[1;31m&\033[0m")}1;END{print "\n\033[1;32mTotal: "c"\033[0m"}'
```

Esto:

- resalta eventos de compilación
- cuenta compilaciones totales
- mejora legibilidad visual

---

## Mostrar únicamente burnouts

```bash
./codexion 100 300 50 50 50 1 50 edf | grep burned
```

---

## Contar burnouts

```bash
./codexion 100 300 50 50 50 1 50 edf | grep -c burned
```

---

## Mostrar únicamente compilaciones

```bash
./codexion 100 300 50 50 50 1 50 edf | grep "is compiling"
```

---

## Estadísticas de compilación por coder

```bash
./codexion 100 300 50 50 50 1 50 edf \
| awk '/is compiling/{count[$2]++} END {for (i in count) print "Coder", i, "compiled", count[i], "times"}'
```

---

## Ordenar coders más activos

```bash
./codexion 100 300 50 50 50 1 50 edf \
| awk '/is compiling/{count[$2]++} END {for (i in count) print count[i], i}' \
| sort -nr
```

---

## Ver únicamente adquisición de dongles

```bash
./codexion 100 300 50 50 50 1 50 edf \
| grep "has taken a dongle"
```

---

## Medir duración total

```bash
time ./codexion 100 300 50 50 50 1 50 edf
```

---

# Blocking Cases Handled

## Deadlock Prevention

El sistema rompe explícitamente la condición de *Circular Wait* de Coffman mediante adquisición asimétrica de recursos:

| Coders pares | Coders impares |
|---|---|
| LEFT → RIGHT | RIGHT → LEFT |

Esto impide ciclos completos de espera.

---

## Starvation Reduction

EDF reduce la probabilidad de starvation priorizando coders cercanos al burnout.

Además:

- se implementa fairness parcial
- existe cooldown de dongles
- se utiliza backoff exponencial

---

## Dongle Cooldown Management

Cada dongle posee un cooldown configurable:

```text
last_used_time + cooldown
```

Esto:

- evita reutilización inmediata
- reduce monopolización
- mejora distribución de recursos

---

## Precise Burnout Detection

El monitor thread verifica constantemente:

```text
current_time - last_compilation
```

La detección ocurre de manera thread-safe mediante `sim_mutex`.

---

## Serialized Logging

Todos los logs utilizan:

```c
pthread_mutex_t log_mutex;
```

Esto evita:

- corrupción de stdout
- interleaving
- mensajes parciales

---

# Thread Synchronization Mechanisms

## pthread_mutex_t

El proyecto utiliza múltiples mutexes especializados:

| Mutex | Función |
|---|---|
| sim_mutex | Estado global |
| log_mutex | Serialización de logs |
| dongle->mutex | Protección del recurso |
| compile_mutex | Coordinación especial |

---

## Protección de Secciones Críticas

Ejemplo:

```c
pthread_mutex_lock(&data->sim_mutex);
coder->compilation_count++;
pthread_mutex_unlock(&data->sim_mutex);
```

Esto evita race conditions sobre:

- compilation_count
- simulation_over
- last_compilation

---

# pthread_cond_t

Cada dongle posee:

```c
pthread_cond_t cond;
```

Los coders esperan eficientemente mediante:

```c
pthread_cond_wait()
```

o:

```c
pthread_cond_timedwait()
```

---

## Comunicación Thread-Safe

El flujo de comunicación es:

```text
coder espera
↓
dongle liberado
↓
broadcast
↓
coder despierta
↓
reevaluación de prioridad
```

---

# Monitor Coordination

El monitor thread:

- supervisa burnout
- detecta finalización global
- despierta todos los hilos bloqueados

Utilizando:

```c
pthread_cond_broadcast()
```

---

# Race Condition Avoidance

Todas las estructuras compartidas están protegidas mediante mutexes:

| Recurso | Protección |
|---|---|
| heap queue | dongle mutex |
| logs | log mutex |
| simulation flags | sim mutex |
| timestamps | sim mutex |

---

# Recursos

## Documentación

- POSIX Threads Programming
- pthreads(7)
- The Little Book of Semaphores
- Operating Systems: Three Easy Pieces
- Advanced Programming in the UNIX Environment

---

## Temas relacionados

- Dining Philosophers Problem
- Deadlock Prevention
- Earliest Deadline First Scheduling
- Condition Variables
- Shared Memory Concurrency
- Priority Scheduling
- Lock Contention
- Starvation Prevention

---

## Recursos online

- https://man7.org/linux/man-pages/man7/pthreads.7.html
- https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_mutex_lock.html
- https://en.wikipedia.org/wiki/Earliest_deadline_first_scheduling
- https://en.wikipedia.org/wiki/Dining_philosophers_problem

---

# Uso de Inteligencia Artificial

Se utilizó IA como herramienta de apoyo técnico para:

- análisis arquitectónico
- revisión conceptual de concurrencia
- documentación técnica
- diseño de scheduling
- explicación teórica de synchronization primitives
- discusión de posibles mejoras futuras

La implementación, debugging, diseño concurrente y desarrollo principal del sistema fueron realizados manualmente.

---

# Posibles Mejoras Futuras

- Aging scheduler
- Priority inheritance
- Lock-free metrics
- Scheduler modular mediante function pointers
- Optimización del broadcast
- Métricas avanzadas de contention
- Estadísticas runtime
- Visualización en tiempo real

---

# Conclusión

Codexion explora problemas reales de concurrencia y scheduling de bajo nivel mediante una simulación multi-threaded completamente sincronizada utilizando `pthreads`.

El proyecto no solo implementa exclusión mutua y comunicación thread-safe, sino también políticas avanzadas de arbitraje y fairness inspiradas en sistemas operativos y runtimes concurrentes reales.
