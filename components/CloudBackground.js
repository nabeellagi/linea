"use client";

import { useEffect, useRef } from "react";
import { Application, Assets, Sprite } from "pixi.js";

const CLOUD = (amount) => {
    let PATHS = []
    for (let i = 1; i <= amount; i++) {
        PATHS.push(`/clouds/cloud${i}.png`)
    }
    return PATHS;
}

const random = (min, max) => Math.random() * (max - min) + min;
const pick = (items) => items[Math.floor(Math.random() * items.length)];

export default function CloudBackground({
    maxClouds = 12,
    minScale = 0.07,
    maxScale = 0.25,
    minSpeed = 8,
    maxSpeed = 35,
    opacity = 0.8,
    interactive = true,
    respectReducedMotion = true,
    cloudAmount = 5
}) {
    const hostRef = useRef(null);

    useEffect(() => {
        let disposed = false;
        let app;
        let onPointerDown;
        let onVisibilityChange;
        let mediaQuery;
        let onMotionChange;

        const host = hostRef.current;
        if (!host) return;

        const clouds = [];
        const textures = [];

        const reducedMotion = window.matchMedia(
            "(prefers-reduced-motion: reduce)"
        );

        const motionDisabled = () =>
            respectReducedMotion && reducedMotion.matches;

        async function initialize() {
            app = new Application();

            await app.init({
                resizeTo: window,
                backgroundAlpha: 0,
                antialias: true,
                autoDensity: true,
                resolution: Math.min(window.devicePixelRatio || 1, 1.5),
                preference: "webgl",
                powerPreference: "low-power",
                hello: false,
            });

            if (disposed) {
                app.destroy({ removeView: true }, {
                    children: true,
                    texture: false,
                });
                return;
            }

            const canvas = app.canvas;

            canvas.style.position = "fixed";
            canvas.style.inset = "0";
            canvas.style.width = "100%";
            canvas.style.height = "100%";
            canvas.style.display = "block";
            canvas.style.pointerEvents = "none";

            host.appendChild(canvas);

            const CLOUD_PATHS = CLOUD(cloudAmount)
            const loadedTextures = await Promise.all(
                CLOUD_PATHS.map((path) => Assets.load(path))
            );

            if (disposed) {
                app.destroy({ removeView: true }, {
                    children: true,
                    texture: false,
                });
                return;
            }

            textures.push(...loadedTextures);

            function spawnCloud(initial = false) {
                if (disposed || !app || clouds.length >= maxClouds) return;

                const texture = pick(textures);
                const sprite = new Sprite(texture);

                const scale = random(minScale, maxScale);
                const direction = Math.random() < 0.5 ? 1 : -1;

                const viewportFactor = Math.min(
                    1,
                    Math.max(0.55, window.innerWidth / 900)
                );

                sprite.anchor.set(0.5);
                sprite.scale.set(scale * viewportFactor);
                sprite.alpha = opacity * random(0.65, 1);
                sprite.eventMode = "none";

                const width = sprite.width;
                const height = sprite.height;
                const screenWidth = app.screen.width;
                const screenHeight = app.screen.height;

                const cloud = {
                    sprite,
                    direction,
                    speed:
                        random(minSpeed, maxSpeed) *
                        (scale / Math.max(maxScale, 0.01)) *
                        direction,
                    baseY: random(height / 2, Math.max(
                        height / 2 + 1,
                        screenHeight - height / 2
                    )),
                    bounceOffset: 0,
                    bounceVelocity: 0,
                    width,
                    height,
                    scale,
                };

                sprite.x = initial
                    ? random(-width, screenWidth + width)
                    : direction > 0
                        ? -width
                        : screenWidth + width;

                sprite.y = cloud.baseY;

                app.stage.addChild(sprite);
                clouds.push(cloud);
            }

            for (let i = 0; i < maxClouds; i++) {
                spawnCloud(true);
            }

            app.ticker.add((ticker) => {
                const dt = Math.min(ticker.deltaMS / 1000, 0.05);
                const screenWidth = app.screen.width;

                for (let i = clouds.length - 1; i >= 0; i--) {
                    const cloud = clouds[i];
                    const sprite = cloud.sprite;

                    if (!motionDisabled()) {
                        sprite.x += cloud.speed * dt;
                    }

                    cloud.bounceVelocity -= cloud.bounceOffset * 20 * dt;
                    cloud.bounceVelocity *= Math.exp(-5 * dt);
                    cloud.bounceOffset += cloud.bounceVelocity * dt;

                    sprite.y = cloud.baseY + cloud.bounceOffset;

                    if (
                        (cloud.direction > 0 &&
                            sprite.x - cloud.width / 2 > screenWidth) ||
                        (cloud.direction < 0 &&
                            sprite.x + cloud.width / 2 < 0)
                    ) {
                        app.stage.removeChild(sprite);
                        sprite.destroy();
                        clouds.splice(i, 1);
                        spawnCloud(false);
                    }
                }
            });

            if (interactive) {
                onPointerDown = (event) => {
                    if (disposed || motionDisabled()) return;

                    const x = event.clientX;
                    const y = event.clientY;

                    let nearest = null;
                    let nearestDistance = Infinity;

                    for (const cloud of clouds) {
                        const dx = cloud.sprite.x - x;
                        const dy = cloud.sprite.y - y;
                        const distance = dx * dx + dy * dy;

                        if (distance < nearestDistance) {
                            nearest = cloud;
                            nearestDistance = distance;
                        }
                    }

                    if (nearest) {
                        nearest.bounceVelocity = -random(180, 300);
                    }
                };

                window.addEventListener("pointerdown", onPointerDown, {
                    passive: true,
                });
            }

            onMotionChange = () => {
                if (!app || disposed) return;

                if (motionDisabled()) {
                    app.ticker.stop();
                } else if (!document.hidden) {
                    app.ticker.start();
                }
            };

            mediaQuery = reducedMotion;
            mediaQuery.addEventListener("change", onMotionChange);

            onVisibilityChange = () => {
                if (!app || disposed) return;

                if (document.hidden || motionDisabled()) {
                    app.ticker.stop();
                } else {
                    app.ticker.start();
                }
            };

            document.addEventListener("visibilitychange", onVisibilityChange);
            onMotionChange();
        }

        initialize().catch((error) => {
            if (!disposed) {
                console.error("Cloud background initialization failed:", error);
            }
        });

        return () => {
            disposed = true;

            if (onPointerDown) {
                window.removeEventListener("pointerdown", onPointerDown);
            }

            if (onVisibilityChange) {
                document.removeEventListener(
                    "visibilitychange",
                    onVisibilityChange
                );
            }

            if (mediaQuery && onMotionChange) {
                mediaQuery.removeEventListener("change", onMotionChange);
            }

            if (app) {
                app.destroy(
                    { removeView: true },
                    { children: true, texture: false }
                );
            }

            clouds.length = 0;
            textures.length = 0;
        };
    }, [
        maxClouds,
        minScale,
        maxScale,
        minSpeed,
        maxSpeed,
        opacity,
        interactive,
        respectReducedMotion,
    ]);

    return (
        <div
            ref={hostRef}
            aria-hidden="true"
            className="pointer-events-none fixed inset-0 z-0 overflow-hidden"
        />
    );
}