#include <assert.h>
#include <stddef.h>

#include "raylib.h"
#include "raymath.h"
#include "gnomecs.h"

/**
 * Simple scroll shooter game with GnomeCS and Raylib.
 * This example was created to show how to use GnomeCS and for testing purposes.
 */

const int screenWidth = 800;
const int screenHeight = 450;
const int columnCount = 8;
const int levelBorder = 50;
const int spawnBorder = levelBorder - 5;
const float timeToSpawnMin = 1;
const float timeToSpawnMax = 3;
const int enemyInWaveMin = 1;
const int enemyInWaveMax = 3;
const float invulnerabilityTime = 3;
const float blinkTime = .25;

enum enemyType {
    enemyType_asteroid,
    enemyType_ship,

    enemyType_count
};

/*
 * Components
 */

_component(inputComp, {
    Vector2 movement;
    bool shoot;
})

_component(transformComp, {
    Vector2 position;
})

_component(movableComp, {
    Vector2 direction;
    float speed;
})

_component(shootableComp, {
    Vector2 direction;
    Vector2 offset;
    float timeToReload;
    float timeAfterShoot;
    bool shoot;
})

_component(enemySpawnerComp, {
    float timeToSpawn;
    float timeAfterSpawn;
    int enemyInWave;
})

_component(enemyComp, {
    int type;
})

_component(circleColliderComp, {
    float radius;
})

_component(collideEventComp, {
    gEntity e1;
    gEntity e2;
})

_component(healthComp, {
    int currentHealth;
    int maxHealth;
})

_component(scoreStorageComp, {
    int score;
})

_component(bulletComp, {
    gEntity owner;
})

_component(lastDamageComp, {
    gEntity from;
})

_component(invulnerabilityComp, {
    float time;
})

_tagComponent(playerTag)

_tagComponent(destroyWhenOffScreenTag)

/*
 * Factory methods
 */

gWorld *createWorld() {
    gWorld *world = gWorldCreate();
    _useWorld(world);
    _addComponentToWorld(inputComp);
    _addComponentToWorld(transformComp);
    _addComponentToWorld(movableComp);
    _addComponentToWorld(playerTag);
    _addComponentToWorld(bulletComp);
    _addComponentToWorld(shootableComp);
    _addComponentToWorld(destroyWhenOffScreenTag);
    _addComponentToWorld(enemySpawnerComp);
    _addComponentToWorld(enemyComp);
    _addComponentToWorld(circleColliderComp);
    _addComponentToWorld(collideEventComp);
    _addComponentToWorld(healthComp);
    _addComponentToWorld(scoreStorageComp);
    _addComponentToWorld(lastDamageComp);
    _addComponentToWorld(invulnerabilityComp);
    _addComponentToWorld(invulnerabilityComp);
    return world;
}

void createPlayer(gWorld *world) {
    _useWorld(world);
    _createEntity(player,
        _componentId(inputComp),
        _componentId(transformComp),
        _componentId(movableComp),
        _componentId(shootableComp),
        _componentId(playerTag),
        _componentId(circleColliderComp),
        _componentId(healthComp),
        _componentId(invulnerabilityComp)
    );

    _useEntity(player);
    transformComp *transform = _take(transformComp);
    movableComp *movable = _take(movableComp);
    shootableComp *shootable = _take(shootableComp);
    circleColliderComp *collider = _take(circleColliderComp);
    healthComp *health = _take(healthComp);

    transform->position = (Vector2){screenWidth / 2, screenHeight / 2};
    movable->direction = (Vector2){0, 0};
    movable->speed = 200;
    shootable->timeToReload = 0.25;
    shootable->direction = (Vector2){0, -1};
    shootable->offset = (Vector2){0, -20};
    collider->radius = 10;
    health->currentHealth = 3;
    health->maxHealth = 3;
}

void createBullet(gWorld *world, const Vector2 position, const Vector2 direction, gEntity owner) {
    _useWorld(world);
    _createEntity(bulletEnity,
        _componentId(transformComp),
        _componentId(movableComp),
        _componentId(bulletComp),
        _componentId(destroyWhenOffScreenTag),
        _componentId(circleColliderComp)
    );

    _useEntity(bulletEnity);
    transformComp *transform = _take(transformComp);
    movableComp *movable = _take(movableComp);
    circleColliderComp *collider = _take(circleColliderComp);
    bulletComp *bullet = _take(bulletComp);

    transform->position = position;
    movable->direction = direction;
    movable->speed = 400;
    collider->radius = 5;
    bullet->owner = owner;
}

void createEnemy(gWorld *world, const Vector2 position, int type) {
    _useWorld(world);

    gEntity enemy = {0};
    float enemySpeed = 0;
    int enemyHealth = 0;

    switch (type) {
        case enemyType_asteroid: {
            _createEntity(asteroid,
                _componentId(transformComp),
                _componentId(movableComp),
                _componentId(destroyWhenOffScreenTag),
                _componentId(enemyComp),
                _componentId(circleColliderComp),
                _componentId(healthComp),
                _componentId(lastDamageComp)
            );
            enemy = asteroid;
            enemySpeed = 100;
            enemyHealth = 1;
        } break;
        case enemyType_ship: {
            _createEntity(ship,
                _componentId(transformComp),
                _componentId(movableComp),
                _componentId(destroyWhenOffScreenTag),
                _componentId(enemyComp),
                _componentId(circleColliderComp),
                _componentId(healthComp),
                _componentId(shootableComp),
                _componentId(lastDamageComp)
            );
            enemy = ship;
            enemySpeed = 50;
            enemyHealth = 3;
            _useEntity(ship);
            shootableComp *shootable = _take(shootableComp);
            shootable->timeToReload = 2;
            shootable->direction = (Vector2){0, 1};
            shootable->offset = (Vector2){0, 50};
            shootable->shoot = true;
        } break;
        default: break;
    }

    _useEntity(enemy);
    transformComp *transform = _take(transformComp);
    movableComp *movable = _take(movableComp);
    enemyComp *enemyData = _take(enemyComp);
    circleColliderComp *collider = _take(circleColliderComp);
    healthComp *health = _take(healthComp);

    transform->position = position;
    movable->direction = (Vector2){0, 1};
    movable->speed = enemySpeed;
    enemyData->type = type;
    collider->radius = 20;
    health->currentHealth = enemyHealth;
    health->maxHealth = enemyHealth;
}

void createSpawner(gWorld *world) {
    _useWorld(world);
    _createEntity(spawner,
        _componentId(enemySpawnerComp)
    );
}

void createScore(gWorld *__cw) {
    _createEntity(score, _componentId(scoreStorageComp));
}

/*
 * Logic Systems
 */

void readInputSystem(gWorld *__cw) {
    _makeQuery
        _with(inputComp)
        _with(playerTag)
    _foreach {
        inputComp *input = _take(inputComp);
        input->movement = (Vector2){0, 0};
        input->shoot = false;

        if (IsKeyDown(KEY_W)) input->movement.y = -1;
        if (IsKeyDown(KEY_S)) input->movement.y = 1;
        if (IsKeyDown(KEY_A)) input->movement.x = -1;
        if (IsKeyDown(KEY_D)) input->movement.x = 1;
        input->movement = Vector2Normalize(input->movement);

        if (IsKeyDown(KEY_SPACE)) input->shoot = true;
    }
}

void moveSystem(gWorld *__cw) {
    const float dt = GetFrameTime();
    _makeQuery
        _with(transformComp)
        _with(movableComp)
    _foreach {
        transformComp *transform = _take(transformComp);
        const movableComp *movable = _take(movableComp);

        const Vector2 velocity = Vector2Scale(movable->direction, movable->speed * dt);
        transform->position = Vector2Add(transform->position, velocity);
    }
}

void shootSystem(gWorld *__cw) {
    _makeQuery
        _with(shootableComp)
        _with(transformComp)
    _foreach {
        shootableComp *shootable = _take(shootableComp);
        transformComp *transform = _take(transformComp);

        shootable->timeAfterShoot += GetFrameTime();
        if (shootable->shoot && shootable->timeAfterShoot > shootable->timeToReload) {
            const Vector2 shootPos = Vector2Add(transform->position, shootable->offset);
            createBullet(__cw, shootPos, shootable->direction, __entity);
            shootable->timeAfterShoot = 0;
        }
    }
}

void applyInputSystem(gWorld *__cw) {
    _makeQuery
        _with(inputComp)
        _with(movableComp)
        _with(shootableComp)
        _with(playerTag)
    _foreach {
        const inputComp *input = _take(inputComp);
        movableComp *movable = _take(movableComp);
        shootableComp *shootable = _take(shootableComp);

        movable->direction = input->movement;
        shootable->shoot = input->shoot;
    }
}

void destroyWhenOffScreenSystem(gWorld *__cw) {
    _makeQuery
        _with(destroyWhenOffScreenTag)
        _with(transformComp)
    _foreach {
        const transformComp *transform = _take(transformComp);
        if (transform->position.y < -levelBorder || transform->position.y > screenHeight + levelBorder ||
            transform->position.x < -levelBorder || transform->position.x > screenWidth + levelBorder) {
            gWorldDestroyEntity(__cw, __entity);
        }
    }
}

void enemySpawnSystem(gWorld *__cw) {
    _makeQuery
        _with(enemySpawnerComp)
    _foreach {
        enemySpawnerComp *spawner = _take(enemySpawnerComp);

        spawner->timeAfterSpawn += GetFrameTime();
        if (spawner->timeAfterSpawn >= spawner->timeToSpawn) {
            for (size_t i = 0; i < spawner->enemyInWave; i++) {
                const int column = screenWidth / columnCount;
                const int x = GetRandomValue(0, columnCount - 1) * column + column / 2;
                const int et = GetRandomValue(0, enemyType_count - 1);
                createEnemy(__cw, (Vector2){x, -spawnBorder}, et);
            }
            spawner->timeAfterSpawn = 0;
            spawner->timeToSpawn = GetRandomValue(timeToSpawnMin, timeToSpawnMax);
            spawner->enemyInWave = GetRandomValue(enemyInWaveMin, enemyInWaveMax);
        }
    }
}

void simpleCollisionDetectionSystem(gWorld *__cw) {
    _makeQuery
        _with(transformComp)
        _with(circleColliderComp)
    gQuery colliders = _saveQuery;
    _foreach {
        const gEntity outEntity = __entity;
        const transformComp *transform1 = _take(transformComp);
        const circleColliderComp *collider1 = _take(circleColliderComp);

        _useQuery(colliders)
        _foreach {
            if (gEntityEq(outEntity, __entity)) continue;

            const transformComp *transform2 = _take(transformComp);
            const circleColliderComp *collider2 = _take(circleColliderComp);

            if (CheckCollisionCircles(transform1->position, collider1->radius, transform2->position, collider2->radius)) {
                _createEntity(collideEvent, _componentId(collideEventComp));
                collideEventComp * ce = gWorldGetComponent(__cw, collideEvent, _componentId(collideEventComp));
                ce->e1 = outEntity;
                ce->e2 = __entity;
            }
        }
    }
}

void applyDamage(gWorld *w, gEntity e, healthComp *h, int value, gEntity from, bool checkInvToDamage) {
    if (checkInvToDamage && gWorldHasComponent(w, from, _componentId(invulnerabilityComp))) {
        invulnerabilityComp *inv = gWorldGetComponent(w, from, _componentId(invulnerabilityComp));
        if (inv->time > 0) return;
    }

    if (gWorldHasComponent(w, e, _componentId(invulnerabilityComp))) {
        invulnerabilityComp *inv = gWorldGetComponent(w, e, _componentId(invulnerabilityComp));
        if (inv->time > 0) return;
        inv->time = invulnerabilityTime;
    }

    h->currentHealth -= value;

    if (gWorldHasComponent(w, e, _componentId(lastDamageComp))) {
        lastDamageComp *lastDamage = gWorldGetComponent(w, e, _componentId(lastDamageComp));
        lastDamage->from = from;
    }
}

void handleCollisionEvents(gWorld *__cw) {
    _makeQuery
        _with(collideEventComp)
    _foreach {
        const collideEventComp *e = _take(collideEventComp);

        bool e1Bullet = gWorldHasComponent(__cw, e->e1, _componentId(bulletComp));
        bool e2Health = gWorldHasComponent(__cw, e->e2, _componentId(healthComp));

        if (e1Bullet && e2Health) {
            gEntity b = e->e1;
            gEntity h = e->e2;

            healthComp *health = gWorldGetComponent(__cw, h, _componentId(healthComp));
            const bulletComp *bullet = gWorldGetComponent(__cw, b, _componentId(bulletComp));
            applyDamage(__cw, h, health, 1, bullet->owner, false);

            gWorldDestroyEntity(__cw, b);
        }

        bool e1Health = gWorldHasComponent(__cw, e->e1, _componentId(enemyComp));

        if (e1Health && e2Health) {
            const gEntity enemy = e->e1;
            const gEntity otherHealth = e->e2;

            healthComp *healthOther = gWorldGetComponent(__cw, otherHealth, _componentId(healthComp));
            healthComp *health = gWorldGetComponent(__cw, enemy, _componentId(healthComp));

            applyDamage(__cw, otherHealth, healthOther, 1, enemy, true);
            applyDamage(__cw, enemy, health, 1, otherHealth, true);
        }
    }
}

void addScoreSystem(gWorld *__cw) {
    _singleEntity(playerEntity, playerTag);
    _single(scoreStorage, scoreStorageComp);

    _makeQuery
        _with(healthComp)
        _with(lastDamageComp)
    _foreach {
        const healthComp *health = _take(healthComp);
        const lastDamageComp *lastDamage = _take(lastDamageComp);

        if (gEntityEq(lastDamage->from, playerEntity) && health->currentHealth <= 0) {
            scoreStorage->score += 1;
        }
    }
}

void destroyDeathEntities(gWorld *__cw) {
    _makeQuery
        _with(healthComp)
    _foreach {
        healthComp *health = _take(healthComp);
        if (health->currentHealth <= 0) {
            gWorldDestroyEntity(__cw, __entity);
        }
    }
}

void cleanCollideEventSystem(gWorld *__cw) {
    _makeQuery
        _with(collideEventComp)
    _foreach {
        gWorldDestroyEntity(__cw, __entity);
    }
}

void invulnerabilitySystem(gWorld *__cw) {
    _makeQuery
        _with(invulnerabilityComp)
    _foreach {
        invulnerabilityComp *inv = _take(invulnerabilityComp);
        inv->time -= GetFrameTime();
    }
}

/*
 * Draw Systems
 */

void drawPlayerSystem(gWorld *__cw) {
    _makeQuery
        _with(transformComp)
        _with(invulnerabilityComp)
        _with(playerTag)
    _foreach {
        const transformComp *transform = _take(transformComp);
        const invulnerabilityComp *inv = _take(invulnerabilityComp);

        bool draw = true;
        if (inv->time > 0) {
            draw = ((int)(inv->time / blinkTime) % 2) == 0;
        }

        if (draw) {
            DrawTriangle(
                Vector2Add(transform->position, (Vector2){0, -20}),
                Vector2Add(transform->position, (Vector2){-10, 10}),
                Vector2Add(transform->position, (Vector2){10, 10}),
                WHITE
            );
        }
    }
}

void drawBulletSystem(gWorld *__cw) {
    _makeQuery
        _with(transformComp)
        _with(bulletComp)
    _foreach {
        const transformComp *transform = _take(transformComp);
        DrawCircleV(transform->position, 5, WHITE);
    }
}

void drawEnemySystem(gWorld *__cw) {
    _makeQuery
        _with(transformComp)
        _with(enemyComp)
    _foreach {
        const transformComp *transform = _take(transformComp);
        const enemyComp *enemyData = _take(enemyComp);

        switch (enemyData->type) {
            case enemyType_asteroid: {
                DrawCircleV(transform->position, 20, WHITE);
            } break;
            case enemyType_ship: {
                DrawTriangle(
                    Vector2Add(transform->position, (Vector2){10, -10}),
                    Vector2Add(transform->position, (Vector2){-10, -10}),
                    Vector2Add(transform->position, (Vector2){0, 20}),
                    WHITE
                );
            } break;
            default: break;
        }
    }
}

void drawPlayerHp(gWorld *__cw) {
    _makeQuery
        _with(healthComp)
        _with(playerTag)
    _foreach {
        const healthComp *health = _take(healthComp);
        DrawText(TextFormat("HP: %d", health->currentHealth), 10, 10, 20, WHITE);
    }
}

void drawScore(gWorld *__cw) {
    _single(score, scoreStorageComp);
    DrawText(TextFormat("Score: %d", score->score), 10, 30, 20, WHITE);
}

/*
 * Game loop and states
 */

enum gameState {
    gameState_init,
    gameState_playing,
    gameState_gameOver
};

int processInitGameState(gWorld **world) {
    gWorld *w = createWorld();
    assert(world != NULL);
    *world = w;
    createPlayer(w);
    createSpawner(w);
    createScore(w);

    BeginDrawing();
    ClearBackground(BLACK);
    EndDrawing();

    return gameState_playing;
}

int processPlayingGameState(gWorld *world) {
    //logic here
    readInputSystem(world);
    applyInputSystem(world);

    moveSystem(world);
    shootSystem(world);
    invulnerabilitySystem(world);

    enemySpawnSystem(world);

    simpleCollisionDetectionSystem(world);
    handleCollisionEvents(world);

    addScoreSystem(world);

    destroyWhenOffScreenSystem(world);
    destroyDeathEntities(world);

    cleanCollideEventSystem(world);

    BeginDrawing();
    ClearBackground(BLACK);
    //draw here
    drawPlayerSystem(world);
    drawBulletSystem(world);
    drawEnemySystem(world);
    drawPlayerHp(world);
    drawScore(world);
    DrawFPS(screenWidth - 80, 10);
    EndDrawing();

    _useWorld(world);
    _singleEntity(player, playerTag);
    if (!gWorldIsEntityAlive(world, player)) return gameState_gameOver;
    return gameState_playing;
}

int processGameOverGameState(gWorld **world) {
    if (*world) {
        gWorldFree(*world);
        *world = NULL;
    }

    BeginDrawing();
    ClearBackground(BLACK);
    DrawText("Game Over", screenWidth / 2 - 50, screenHeight / 2 - 10, 20, WHITE);
    DrawText("Press any key to restart", screenWidth / 2 - 100, screenHeight / 2 + 10, 20, WHITE);
    EndDrawing();

    if (GetKeyPressed() > 0) return gameState_init;
    return gameState_gameOver;
}

int main() {

    int currentGameState = gameState_init;

    InitWindow(screenWidth, screenHeight, "GnomeCS Scroll Shooter");
    SetTargetFPS(60);

    gWorld *world = NULL;

    while (!WindowShouldClose()) {

        switch (currentGameState) {
            case gameState_init:
                currentGameState = processInitGameState(&world);
                break;
            case gameState_playing:
                currentGameState = processPlayingGameState(world);
                break;
            case gameState_gameOver:
                currentGameState = processGameOverGameState(&world);
                break;
            default:
                return 1;
        }
    }

    CloseWindow();

    return 0;
}