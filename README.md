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

Prioriza al coder que llegó primero a la cola de espera. El timestamp del ticket se calcula **una sola vez** al inicio de cada ciclo de compilación y se mantiene constante durante todos los reintentos, garantizando orden justo de llegada.

### EDF (Earliest Deadline First)

Prioriza al coder más cercano al burnout utilizando deadlines dinámicos:

```text
deadline = last_compilation + time_to_burnout
```

El deadline también se calcula **una sola vez** por ciclo y se propaga a todos los intentos de adquisición, evitando que reintentos tardíos penalicen al coder con menor prioridad real.

---

## Priority Queue basada en Min Heap

Cada dongle mantiene una cola de prioridad independiente:

```c
t_heap queue;
```

La prioridad se calcula mediante:

- timestamp (FIFO: tiempo de llegada / EDF: deadline)
- coder_id (tie-break)

### Complejidades

| Operación | Complejidad |
|---|---|
| Insert | O(log n) |
| Extract Min | O(log n) |
| Peek | O(1) |

---

## Backoff Exponencial

Para reducir contention y retry storms cuando el segundo dongle no está disponible:

```text
200µs → 400µs → 800µs → ... → 8000µs (máximo)
```

El backoff es único por coder basado en su id:

```text
b = 200 + (coder_id * 137) % 400
```

Esto evita sincronización accidental entre hilos y mejora fairness.

---

## Adquisición atómica de dos recursos

La adquisición de los dos dongles sigue el patrón try-and-release:

```text
1. wait_for_dongle(primero)   ← bloquea hasta obtenerlo
2. try_take_dongle(segundo)   ← intento no bloqueante
3. si falla → release(primero) + backoff + reintentar
```

Esto elimina el hold-and-wait — condición necesaria para deadlock.

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
./codexion 5 1200 200 100 100 7 50 edf
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
./codexion 5 100 200 100 100 7 50 fifo | grep burned
```

---

## Contar burnouts

```bash
./codexion 5 100 200 100 100 7 50 fifo | grep -c burned
```

---

## Mostrar únicamente compilaciones

```bash
./codexion 5 1200 200 100 100 7 50 edf | grep "is compiling"
```

---

## Estadísticas de compilación por coder

```bash
./codexion 5 1200 200 100 100 7 50 edf \
| awk '/is compiling/{count[$2]++} END {for (i in count) print "Coder", i, "compiled", count[i], "times"}'
```

---

## Ordenar coders más activos

```bash
./codexion 5 1200 200 100 100 7 50 edf \
| awk '/is compiling/{count[$2]++} END {for (i in count) print count[i], i}' \
| sort -nr
```

---

## Ver únicamente adquisición de dongles

```bash
./codexion 5 1200 200 100 100 7 50 edf \
| grep "has taken a dongle"
```

---

## Medir duración total

```bash
time ./codexion 5 1200 200 100 100 7 50 edf
```

---

# Blocking Cases Handled

## Deadlock Prevention

El sistema elimina la condición de *Hold and Wait* de Coffman mediante el patrón try-and-release:

```text
1. Adquirir primer dongle (bloqueante)
2. Intentar adquirir segundo dongle (no bloqueante)
3. Si falla → liberar el primero inmediatamente
4. Backoff exponencial → reintentar
```

Nunca se retienen recursos mientras se espera otro. Adicionalmente, la asignación asimétrica rompe posibles ciclos:

| Coders pares | Coders impares |
|---|---|
| LEFT → RIGHT | RIGHT → LEFT |

---

## Starvation Reduction

EDF reduce la probabilidad de starvation priorizando coders cercanos al burnout.

El priority ticket se calcula una sola vez por ciclo — los reintentos no penalizan al coder con deadline más urgente.

Además:

- se implementa fairness mediante heap de prioridad por dongle
- existe cooldown de dongles para distribución equitativa
- el monitor hace broadcast periódico cada 2ms para despertar coders bloqueados por cooldown

---

## Dongle Cooldown Management

Cada dongle posee un cooldown configurable verificado en `dongle_is_acquirable`:

```text
elapsed = now - last_used_time
si elapsed < dongle_cooldown → no disponible
```

El monitor hace `broadcast_all_dongles` periódicamente para despertar a los coders que esperan a que el cooldown expire, evitando esperas indefinidas sin `pthread_cond_timedwait`.

---

## Precise Burnout Detection

El monitor thread verifica constantemente con un ciclo de 2ms:

```text
current_time - last_compilation > time_to_burnout → burnout
```

La detección ocurre de manera thread-safe mediante `sim_mutex`. El mensaje de burnout se imprime dentro de los 10ms requeridos.

---

## Serialized Logging

Todos los logs utilizan orden fijo de adquisición de mutexes:

```c
pthread_mutex_lock(&data->sim_mutex);
pthread_mutex_lock(&data->log_mutex);
// imprimir
pthread_mutex_unlock(&data->log_mutex);
pthread_mutex_unlock(&data->sim_mutex);
```

El orden `sim_mutex → log_mutex` es consistente en todos los hilos, eliminando deadlocks cruzados entre monitor y coders.

---

# Thread Synchronization Mechanisms

## pthread_mutex_t

El proyecto utiliza mutexes especializados con orden de adquisición estricto:

| Mutex | Función | Orden |
|---|---|---|
| sim_mutex | Estado global de simulación | 1º |
| log_mutex | Serialización de logs | 2º |
| dongle->mutex | Protección del recurso y su heap | independiente |

El orden `sim_mutex → log_mutex` nunca se invierte, eliminando deadlocks cruzados detectados por helgrind.

---

## Protección de Secciones Críticas

```c
pthread_mutex_lock(&data->sim_mutex);
coder->compilation_count++;
pthread_mutex_unlock(&data->sim_mutex);
```

Campos protegidos por `sim_mutex`:

- `compilation_count`
- `simulation_over`

Campos de escritura exclusiva (sin mutex necesario):

- `last_compilation` — solo escribe el hilo propietario

---

## pthread_cond_t

Cada dongle posee:

```c
pthread_cond_t cond;
```

Los coders esperan eficientemente mediante `pthread_cond_wait` exclusivamente — se eliminó `pthread_cond_timedwait` por producir falsos positivos en helgrind. El monitor suple esta funcionalidad haciendo broadcast periódico.

---

## Comunicación Thread-Safe

El flujo de comunicación es:

```text
coder inserta ticket en heap del dongle
↓
coder espera con pthread_cond_wait
↓
dongle liberado → release_dongle → broadcast
o monitor → broadcast_all_dongles cada 2ms
↓
coder despierta
↓
reevaluación: dongle_is_acquirable()
↓
si es el primero del heap y cooldown pasó → toma el dongle
si no → vuelve a cond_wait
```

---

# Monitor Coordination

El monitor thread:

- supervisa burnout de cada coder cada 2ms
- detecta finalización global cuando todos completaron `number_of_compiles_required`
- despierta todos los hilos bloqueados con `broadcast_all_dongles`
- imprime el mensaje de burnout con `log_mutex` sin tener `sim_mutex` simultáneamente

---

# Race Condition Avoidance

Todas las estructuras compartidas están protegidas con orden consistente:

| Recurso | Protección |
|---|---|
| heap queue | dongle->mutex (siempre tomado al operar el heap) |
| logs | sim_mutex → log_mutex (orden fijo) |
| simulation_over | sim_mutex |
| compilation_count | sim_mutex |
| last_compilation | escritura exclusiva del hilo propietario |
| available, last_used_time | dongle->mutex |

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
- Deadlock Prevention (condiciones de Coffman)
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
- detección y diagnóstico de race conditions con helgrind
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
