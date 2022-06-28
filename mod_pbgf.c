/* thread.c */

static struct list sleep_list;			//Lista de todas as threads no estado de SLEEP (Bloqueada)

void thread_sleep(int64_t ticks){                     //se a thread atual não for a idle thread, troque o estado da thread chamada para BLOQUEADA

  struct thread *cur = thread_current();             //cur vai ser um ponteiro do tipo struct thread que vau receber a thread atual
  enum intr_level previus_interrup;                  //variavel do tipo intr_level para guardar depois o estado da interrupção

  previus_interrup = intr_disable();              //vai desligar a interrupção guardando em old_level a situação anterior
  cur = thread_current();                  //cur vai receber a thread atual
  ASSERT (cur != idle_thread);             //vai tentar ver se a thread atual é igual a idle thread
  cur->wake_time = ticks;                  //wake_time da thread vai receber o numero de ticks atual para poder tirar ela de bloquada depois
  list_push_back(&sleep_list, &cur->elem); //vai adicionar no fim da fila sleep_list
  thread_current ()->status = THREAD_BLOCKED; //troca o estado da thread para bloqueada

  schedule ();                                //chama o escalonador
  intr_set_level(previus_interrup);               //habilita interrupção
}

void thread_wakeup (int64_t ticks) {
  struct list_elem *begin = list_begin(&sleep_list);           //inicializa uma struct do tipo list_elem como ponteiro que vai receber o começo da sleeplist

  while(begin != list_end(&sleep_list)) {                      //enquanto o começo for diferente do fim da fila:
    struct thread *t = list_entry(begin, struct thread, elem); //chama a função entry que converte um ponteiro de um elemento da lista para um ponteiro do  tipo lista em que o elemento está
    if (t->wake_time <= ticks) {                               //se o wake_time for menor ou igual ao numero de ticks:
      begin = list_remove (begin);                             //o começo vai receber o proximo elemento do que foi removido do começo da fila
      ASSERT (t->status == THREAD_BLOCKED);                    // vai testar se o status da thread t é bloqueado
      list_push_back (&ready_list, &t->elem);                  //então vai adicionar no fim da ready_list, que é a fila das threads que estão prontas
      t->status = THREAD_READY;                               // altera o status da thread para Ready

    } else {                                                   //se não, begin vai receber o proximo item da lista e vai voltar para i loop até a lista ser varrida completamente
      begin = list_next(begin);
    }
  }
}

void
thread_init (void) 
{
  ASSERT (intr_get_level () == INTR_OFF);

  lock_init (&tid_lock);
  list_init (&ready_list);
  list_init (&all_list);
  list_init (&sleep_list);                 //declaração da sleep_list

  /* Set up a thread structure for the running thread. */
  initial_thread = running_thread ();
  init_thread (initial_thread, "main", PRI_DEFAULT);
  initial_thread->status = THREAD_RUNNING;
  initial_thread->tid = allocate_tid ();
}

/* timer.c */

static void
timer_interrupt (struct intr_frame *args UNUSED)
{
  ticks++;
  thread_tick ();
  //checar a lista de threads dormindo e o tick global, achar alguma thread para acordar e mover elas para lista de ready se necessario
  thread_wakeup(ticks);
}

void
timer_sleep (int64_t ticks) 
{
  int64_t start_t = timer_ticks();        //start_t vai receber o numero de ticks desde o inicio
  signed long int sum = start_t + ticks;  //declarei uma variavel do tipo signed long int, que vai receber o numero de ticks desde o inicio mais o numero de ticks
  ASSERT (intr_get_level () == INTR_ON);  //esse comando vai testar se as interrupções estão ligados
  if (timer_elapsed (start_t) < ticks){   //se o tempo que passou for menor que o numero de ticks
    thread_sleep(sum);			   //chamada da função sleep
  }
}
