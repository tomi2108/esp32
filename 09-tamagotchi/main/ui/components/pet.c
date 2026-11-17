#include "pet.h"
#include "portmacro.h"

#define PHRASES 36
#define HUNGRY_PHRASES 12
#define PHRASE_DURATION 5000
#define TALKING_MELODIES 6
#define NOTES_PER_MELODY 4
#define HUNGRY_MELODIES 4

extern OLED oled;
extern int wholenote;
extern PassiveBuzzer buzzer;

static const Note talking[TALKING_MELODIES][NOTES_PER_MELODY] = {
    {
        {380, SIXTEENTH},
        {440, SIXTEENTH},
        {410, EIGHTH},
        {350, EIGHTH},
    },
    {
        {450, SIXTEENTH},
        {400, EIGHTH},
        {480, SIXTEENTH},
        {420, EIGHTH},
    },
    {
        {360, EIGHTH},
        {430, SIXTEENTH},
        {490, SIXTEENTH},
        {400, EIGHTH},
    },
    {
        {480, SIXTEENTH},
        {450, SIXTEENTH},
        {390, EIGHTH},
        {370, EIGHTH},
    },
    {
        {420, SIXTEENTH},
        {500, SIXTEENTH},
        {460, EIGHTH},
        {0, 0},
    },
    {
        {520, SIXTEENTH},
        {460, SIXTEENTH},
        {410, EIGHTH},
        {0, 0},
    },
};

static const Note hungry_talking[HUNGRY_MELODIES][NOTES_PER_MELODY] = {
    {
        {260, EIGHTH},
        {230, SIXTEENTH},
        {210, EIGHTH},
        {180, QUARTER},
    },
    {
        {220, SIXTEENTH},
        {250, EIGHTH},
        {200, EIGHTH},
        {180, QUARTER},
    },
    {
        {280, EIGHTH},
        {240, EIGHTH},
        {190, QUARTER},
        {0, 0},
    },
    {
        {200, EIGHTH},
        {180, SIXTEENTH},
        {220, EIGHTH},
        {170, QUARTER},
    },
};

static const char *hungry_phrases[HUNGRY_PHRASES] = {
    "Tengo hambre!", "Quiero comer!",   "Dame comidita",   "Mi pancita!",
    "Algo rico?",    "Tengo hambre :(", "Quiero aguita!",  "Alimentame!",
    "Un bocadito?",  "Mi pancita ruge", "Necesito comida", "Porfis, comida!",
};

static const char *phrases[PHRASES] = {
    "Tu puedes!",         "Vamos juntos!",    "Sigue brillando!",
    "Un dia a la vez",    "No te rindas!",    "Todo es posible!",
    "Eres genial!",       "Confia en ti!",    "Pequenos pasos!",
    "Hoy sera lindo!",    "Respira profundo", "Toma aguita!",
    "Descansa un poco",   "Cuidate mucho",    "Sonrie!",
    "Todo esta bien",     "Date un mimo",     "Pausa y respira!",
    "Escucha tu corazon", "Quierete mucho!",  "Te quiero!",
    "Estoy contigo!",     "Que lindo dia!",   "Eres especial!",
    "Me haces feliz!",    "Vamos a jugar!",   "Un abracito!",
    "Brillas mucho!",     "Aqui estoy!",      "Somos equipo!",
    "Buen dia!",          "A disfrutar!",     "Que bonito!",
    "Dulces suenos!",     "A descansar!",     "Manana sera mejor!",
};

static void talking_render(Overlay *self) {
  char *phrase = (char *)self->data;
  oled_write_text(&oled, 30, 20, false, phrase);
}

Overlay talking_overlay = {.render = talking_render};

void hide_talking(void *data) {
  vTaskDelay(pdMS_TO_TICKS(PHRASE_DURATION));
  overlay_remove(&talking_overlay);
}

static TickType_t next_talking = 0;

void talk(TickType_t now, State state, int hunger) {
  overlay_show(&talking_overlay);
  const char *phrase;
  Melody melody;

  if (state.hunger < hunger) {
    int p = random_range(0, HUNGRY_PHRASES - 1);
    int t = random_range(0, HUNGRY_MELODIES - 1);
    phrase = hungry_phrases[p];
    melody = (Melody){.notes = hungry_talking[t], .count = NOTES_PER_MELODY};
  } else {
    int p = random_range(0, PHRASES - 1);
    int t = random_range(0, TALKING_MELODIES - 1);

    phrase = phrases[p];
    melody = (Melody){.notes = talking[t], .count = NOTES_PER_MELODY};
  }

  char *data = strdup(phrase);
  talking_overlay.data = data;
  buzzer_play_melody(buzzer, wholenote, melody);
  task_start("talking_overlay", hide_talking, data);
  next_talking = now + pdMS_TO_TICKS(random_range(15000, 30000));
}

void pet_sprite_render() {
  TickType_t now = xTaskGetTickCount();
  State state = state_get();
  if (next_talking == 0)
    next_talking = now + pdMS_TO_TICKS(15000);

  if (now >= next_talking && state.status != PET_SLEEPING)
    talk(now, state, 30);

  Animation *animation = animations_pet_status(state.species, state.status);

  oled_draw_bitmap(&oled, 0, 10, animation->width, animation->height, false,
                   animation_frame(animation));
}
