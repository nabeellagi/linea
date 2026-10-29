import { useMemo, useRef, useState, useId, useEffect } from "react";
import WoodPanel from "./WoodPanel"
import { H2 } from "./Heading"

const WIDTH = 800;
const HEIGHT = 460;
const PLOT_LEFT = 54;
const PLOT_RIGHT = 786;
const PLOT_TOP = 18;
const PLOT_BOTTOM = 420;
const PLOT_WIDTH = PLOT_RIGHT - PLOT_LEFT;
const PLOT_HEIGHT = PLOT_BOTTOM - PLOT_TOP;
const PLOT_CX = (PLOT_LEFT + PLOT_RIGHT) / 2;
const PLOT_CY = (PLOT_TOP + PLOT_BOTTOM) / 2;
const MIN_SCALE = 0.05;
const MAX_SCALE = 140;

function clamp(value, min, max) {
    return Math.min(max, Math.max(min, value));
}

function parseCoordinatePairs(input) {
    const pairPattern = /\(\s*([+-]?(?:\d+(?:\.\d*)?|\.\d+))\s*,\s*([+-]?(?:\d+(?:\.\d*)?|\.\d+))\s*\)/g;
    const points = [];
    let match;

    while ((match = pairPattern.exec(input)) !== null) {
        const x = Number(match[1]);
        const y = Number(match[2]);

        if (Number.isFinite(x) && Number.isFinite(y)) {
            points.push({
                x,
                y,
                xLabel: match[1],
                yLabel: match[2],
            });
        }
    }

    return points;
}

function calculateFit(points) {
    if (points.length < 2) return null;

    const meanX = points.reduce((sum, point) => sum + point.x, 0) / points.length;
    const meanY = points.reduce((sum, point) => sum + point.y, 0) / points.length;

    let numerator = 0;
    let denominator = 0;

    for (const point of points) {
        const dx = point.x - meanX;
        numerator += dx * (point.y - meanY);
        denominator += dx * dx;
    }

    if (!Number.isFinite(denominator) || denominator === 0) return null;

    const slope = numerator / denominator;
    const intercept = meanY - slope * meanX;

    if (!Number.isFinite(slope) || !Number.isFinite(intercept)) return null;
    return { intercept, slope };
}

function initialView(points, fit) {
    if (points.length === 0) {
        return { centerX: 0, centerY: 0, scale: 36 }
    }

    const xs = points.map((point) => point.x);
    const ys = points.map((point) => point.y);
    const minDataX = Math.min(...xs);
    const maxDataX = Math.max(...xs);

    if (fit) {
        ys.push(fit.intercept + fit.slope * minDataX);
        ys.push(fit.intercept + fit.slope * maxDataX);
    }

    let minX = minDataX;
    let maxX = maxDataX;
    let minY = Math.min(...ys);
    let maxY = Math.max(...ys);

    const padX = Math.max((maxX - minX) * 0.18, 0.7);

    const padY = Math.max((maxY - minY) * 0.18, 0.7);
    minX -= padX;
    maxX += padX;
    minY -= padY;
    maxY += padY;

    const spanX = Math.max(maxX - minX, 1);
    const spanY = Math.max(maxY - minY, 1);
    const scale = clamp(
        Math.min(PLOT_WIDTH / spanX, PLOT_HEIGHT / spanY, MAX_SCALE),
        MIN_SCALE,
        MAX_SCALE,
    )

    return {
        centerX: (minX + maxX) / 2,
        centerY: (minY + maxY) / 2,
        scale,
    };
}

function niceGridStep(scale) {
    const rawStep = 56 / scale;
    const power = 10 ** Math.floor(Math.log10(rawStep));
    const fraction = rawStep / power;
    const niceFraction = fraction <= 1 ? 1 : fraction <= 2 ? 2 : fraction <= 5 ? 5 : 10;
    return niceFraction * power;
}

function makeTicks(min, max, step) {
    if (!Number.isFinite(min) || !Number.isFinite(max) || !Number.isFinite(step) || step <= 0) {
        return [];
    }

    const first = Math.ceil(min / step) * step;
    const count = Math.min(80, Math.max(0, Math.floor((max - first) / step) + 1));

    return Array.from({ length: count }, (_, index) => {
        const value = first + index * step;
        return Object.is(value, -0) ? 0 : Number(value.toPrecision(10));
    });
}

function formatNumber(value) {
    if (!Number.isFinite(value)) return String(value);
    if (Object.is(value, -0) || value === 0) return "0";
    return Number(value.toPrecision(6)).toString();
}

function equationLabel(fit) {
    if (!fit) return "Fit line unavailable";
    if (Math.abs(fit.slope) < 1e-12) return `ŷ = ${formatNumber(fit.intercept)}`;

    const sign = fit.slope < 0 ? "−" : "+";
    return `ŷ = ${formatNumber(fit.intercept)} ${sign} ${formatNumber(Math.abs(fit.slope))}x`;
}

export default function LinearRegressionGraph({ input }) {
    const clipId = useId().replace(/:/g, "");
    const svgRef = useRef(null);
    const dragRef = useRef(null);
    const points = useMemo(() => parseCoordinatePairs(input), [input]);
    const fit = useMemo(() => calculateFit(points), [points]);
    const initial = useMemo(() => initialView(points, fit), [points, fit]);
    const [view, setView] = useState(initial);
    const [activePointIndex, setActivePointIndex] = useState(null);
    const [isDragging, setIsDragging] = useState(false);

    const visible = useMemo(() => {
        const halfWidth = PLOT_WIDTH / (2 * view.scale);
        const halfHeight = PLOT_HEIGHT / (2 * view.scale);
        const left = view.centerX - halfWidth;
        const right = view.centerX + halfWidth;
        const bottom = view.centerY - halfHeight;
        const top = view.centerY + halfHeight;
        const step = niceGridStep(view.scale);

        return {
            left,
            right,
            bottom,
            top,

            xTicks: makeTicks(left, right, step),
            yTicks: makeTicks(bottom, top, step),

        };
    }, [view]);

    const mapX = (x) => PLOT_CX + (x - view.centerX) * view.scale;
    const mapY = (y) => PLOT_CY - (y - view.centerY) * view.scale;

    function pointerPosition(event) {
        const rect = svgRef.current?.getBoundingClientRect();
        if (!rect || rect.width === 0 || rect.height === 0) return null;

        return {
            x: ((event.clientX - rect.left) / rect.width) * WIDTH,
            y: ((event.clientY - rect.top) / rect.height) * HEIGHT,
        };
    }

    function handlePointerDown(event) {
        if (event.target?.closest?.("[data-point-index]")) return;
        if (event.button !== undefined && event.button !== 0) return;

        const position = pointerPosition(event);
        if (!position) return;

        dragRef.current = {
            pointerId: event.pointerId,
            clientX: event.clientX,
            clientY: event.clientY,
            centerX: view.centerX,
            centerY: view.centerY,
            scale: view.scale,
        };
        setIsDragging(true);
        event.currentTarget.setPointerCapture?.(event.pointerId);
    }

    function handlePointerMove(event) {
        const drag = dragRef.current;
        const rect = svgRef.current?.getBoundingClientRect();
        if (!drag || !rect || rect.width === 0 || rect.height === 0) return;

        const dx = ((event.clientX - drag.clientX) / rect.width) * WIDTH;
        const dy = ((event.clientY - drag.clientY) / rect.height) * HEIGHT;

        setView({
            centerX: drag.centerX - dx / drag.scale,
            centerY: drag.centerY + dy / drag.scale,
            scale: drag.scale,
        });
    }

    function handlePointerUp() {
        dragRef.current = null;
        setIsDragging(false);
    }

    function zoom(factor) {
        setView((current) => ({
            ...current,
            scale: clamp(current.scale * factor, MIN_SCALE, MAX_SCALE),
        }));

    }

    const activePoint = activePointIndex === null ? null : points[activePointIndex];



    useEffect(() => {
        const svg = svgRef.current;
        if (!svg) return;

        function handleNativeWheel(event) {
            if (event.cancelable) {
                event.preventDefault();
            }

            const rect = svg.getBoundingClientRect();

            if (rect.width === 0 || rect.height === 0) return;

            const position = {
                x: ((event.clientX - rect.left) / rect.width) * WIDTH,
                y: ((event.clientY - rect.top) / rect.height) * HEIGHT,
            };

            const factor = Math.exp(-event.deltaY * 0.0012);

            setView((current) => {
                const nextScale = clamp(
                    current.scale * factor,
                    MIN_SCALE,
                    MAX_SCALE
                );

                const worldX =
                    current.centerX + (position.x - PLOT_CX) / current.scale;

                const worldY =
                    current.centerY - (position.y - PLOT_CY) / current.scale;

                return {
                    scale: nextScale,
                    centerX:
                        worldX - (position.x - PLOT_CX) / nextScale,
                    centerY:
                        worldY + (position.y - PLOT_CY) / nextScale,
                };
            });
        }
        svg.addEventListener("wheel", handleNativeWheel, {
            passive: false,
        });
        return () => {
            svg.removeEventListener("wheel", handleNativeWheel);
        };
    }, []);


    return (
        <section className="mx-auto mt-7 w-full min-w-0 max-w-5xl" aria-labelledby="linea-graph-title">
            <div className="mb-3 px-1">
                <p className="font-comic-relief text-xs font-bold uppercase tracking-[0.18em] text-[#185267]">
                    Visualisation
                </p>
                <H2 id="linea-graph-title" className="text-[#e2ffb8] [text-shadow:0_4px_12px_rgba(0,0,0,0.4)] [-webkit-text-stroke:1px_#0785a8]">
                    Regression graph
                </H2>
                <p className="mt-1 font-comic-relief text-sm text-[#185267]">
                    Drag to pan infinitely. Scroll to zoom, or use the controls on mobile.
                </p>
            </div>

            <WoodPanel width="100%" className="min-w-0 max-w-5xl" borderColor="#7b3607" textureScale={2}>
                <div className="min-w-0 p-3 sm:p-5">
                    <div className="mb-3 flex flex-wrap items-center justify-between gap-2 rounded-lg border border-[#dbc69b] bg-[#fff7df] px-3 py-2 text-[#553019]">
                        <p className="min-w-0 wrap-break-word font-comic-relief text-sm font-bold sm:text-base">
                            {equationLabel(fit)}
                        </p>
                        <div className="flex shrink-0 items-center gap-2" aria-label="Graph zoom controls">
                            <button
                                type="button"
                                onClick={() => zoom(1.25)}
                                aria-label="Zoom in"
                                className="size-9 rounded-lg border border-[#b89a68] bg-[#fff0c7] font-gochi-hand text-xl text-[#553019] hover:bg-white focus-visible:outline-2 focus-visible:outline-[#0785a8]"
                            >
                                +
                            </button>
                            <button
                                type="button"
                                onClick={() => zoom(0.8)}
                                aria-label="Zoom out"
                                className="size-9 rounded-lg border border-[#b89a68] bg-[#fff0c7] font-gochi-hand text-xl text-[#553019] hover:bg-white focus-visible:outline-2 focus-visible:outline-[#0785a8]"
                            >
                                −
                            </button>
                            <button
                                type="button"
                                onClick={() => {
                                    setView(initial);
                                    setActivePointIndex(null);
                                }}
                                className="min-h-9 rounded-lg border border-[#b89a68] bg-[#fff0c7] px-3 font-comic-relief text-xs font-bold text-[#553019] hover:bg-white focus-visible:outline-2 focus-visible:outline-[#0785a8] sm:text-sm"
                            >
                                Reset
                            </button>
                        </div>
                    </div>

                    <div className="mb-3 flex min-h-10 items-center rounded-lg bg-[#fffaf0] px-3 py-2 font-comic-relief text-sm text-[#553019]" role="status" aria-live="polite">
                        {activePoint ? (
                            <span>
                                <strong>Point {activePointIndex + 1}:</strong> ({activePoint.xLabel}, {activePoint.yLabel})
                            </span>
                        ) : (
                            <span>Hover over a point or tap it to inspect its exact input coordinates.</span>
                        )}
                    </div>

                    <div className="w-full overflow-hidden rounded-lg border border-[#d8c39b] bg-[#fffaf0]">
                        <svg
                            ref={svgRef}
                            viewBox={`0 0 ${WIDTH} ${HEIGHT}`}
                            className="block h-auto w-full select-none"
                            role="group"
                            aria-label="Interactive Cartesian graph of input points and the fitted regression line"
                            style={{
                                touchAction: "none",
                                cursor: isDragging ? "grabbing" : "grab",
                            }}
                            onPointerDown={handlePointerDown}
                            onPointerMove={handlePointerMove}
                            onPointerUp={handlePointerUp}
                            onPointerCancel={handlePointerUp}
                            onLostPointerCapture={handlePointerUp}
                        >
                            <title>Interactive regression graph. Drag to pan; use the zoom buttons on touch screens.</title>
                            <defs>
                                <clipPath id={clipId}>
                                    <rect
                                        x={PLOT_LEFT}
                                        y={PLOT_TOP}
                                        width={PLOT_WIDTH}
                                        height={PLOT_HEIGHT}
                                    />
                                </clipPath>
                            </defs>

                            <rect x="0" y="0" width={WIDTH} height={HEIGHT} fill="#fffaf0" />

                            <rect
                                x={PLOT_LEFT}
                                y={PLOT_TOP}
                                width={PLOT_WIDTH}
                                height={PLOT_HEIGHT}
                                fill="#fffdf7"
                                stroke="#d8c39b"
                                strokeWidth="1"
                            />

                            <g clipPath={`url(#${clipId})`}>
                                {visible.xTicks.map((value, index) => {
                                    const x = mapX(value);
                                    return (
                                        <line
                                            key={`vertical-${index}`}
                                            x1={x}
                                            y1={PLOT_TOP}
                                            x2={x}
                                            y2={PLOT_BOTTOM}
                                            stroke="#eadfc9"
                                            strokeWidth="1"
                                        />
                                    );
                                })}

                                {visible.yTicks.map((value, index) => {
                                    const y = mapY(value);
                                    return (
                                        <line
                                            key={`horizontal-${index}`}
                                            x1={PLOT_LEFT}
                                            y1={y}
                                            x2={PLOT_RIGHT}
                                            y2={y}
                                            stroke="#eadfc9"
                                            strokeWidth="1"
                                        />
                                    );
                                })}

                                {visible.bottom <= 0 && visible.top >= 0 && (
                                    <line
                                        x1={PLOT_LEFT}
                                        y1={mapY(0)}
                                        x2={PLOT_RIGHT}
                                        y2={mapY(0)}
                                        stroke="#7c6245"
                                        strokeWidth="1.8"
                                    />
                                )}

                                {visible.left <= 0 && visible.right >= 0 && (
                                    <line
                                        x1={mapX(0)}
                                        y1={PLOT_TOP}
                                        x2={mapX(0)}
                                        y2={PLOT_BOTTOM}
                                        stroke="#7c6245"
                                        strokeWidth="1.8"
                                    />
                                )}
                                {fit && (
                                    <line
                                        x1={PLOT_LEFT}
                                        y1={mapY(fit.intercept + fit.slope * visible.left)}
                                        x2={PLOT_RIGHT}
                                        y2={mapY(fit.intercept + fit.slope * visible.right)}
                                        stroke="#0785a8"
                                        strokeWidth="3.5"
                                        strokeLinecap="round"
                                        vectorEffect="non-scaling-stroke"
                                    />
                                )}

                                {points.map((point, index) => {
                                    const selected = activePointIndex === index;
                                    return (
                                        <circle
                                            key={`point-${index}`}
                                            data-point-index={index}
                                            cx={mapX(point.x)}
                                            cy={mapY(point.y)}
                                            r={selected ? 7.5 : 5.5}
                                            fill={selected ? "#e88a24" : "#7b3607"}
                                            stroke="#fffaf0"
                                            strokeWidth="2"
                                            tabIndex={0}
                                            role="button"
                                            aria-label={`Point ${index + 1}: (${point.xLabel}, ${point.yLabel})`}
                                            aria-pressed={selected}
                                            style={{ cursor: "pointer", outline: "none" }}
                                            onPointerDown={(event) => {
                                                event.stopPropagation();
                                                setActivePointIndex(index);
                                            }}
                                            onPointerEnter={() => setActivePointIndex(index)}
                                            onFocus={() => setActivePointIndex(index)}
                                            onClick={(event) => {
                                                event.stopPropagation();
                                                setActivePointIndex(index);
                                            }}
                                        >
                                            <title>{`Point ${index + 1}: (${point.xLabel}, ${point.yLabel})`}</title>
                                        </circle>
                                    );
                                })}
                            </g>

                            {visible.xTicks.map((value, index) => (
                                <text
                                    key={`xlabel-${index}`}
                                    x={mapX(value)}
                                    y={PLOT_BOTTOM + 20}
                                    fill="#725b41"
                                    fontSize="12"
                                    textAnchor="middle"
                                >
                                    {formatNumber(value)}
                                </text>
                            ))}

                            {visible.yTicks.map((value, index) => (
                                <text
                                    key={`ylabel-${index}`}
                                    x={PLOT_LEFT - 8}
                                    y={mapY(value) + 4}
                                    fill="#725b41"
                                    fontSize="12"
                                    textAnchor="end"
                                >
                                    {formatNumber(value)}
                                </text>
                            ))}

                            <text x={PLOT_RIGHT - 2} y={HEIGHT - 5} fill="#553019" fontSize="13" textAnchor="end" fontWeight="600">
                                x
                            </text>
                            <text x={12} y={PLOT_TOP + 2} fill="#553019" fontSize="13" fontWeight="600">
                                y
                            </text>
                        </svg>
                    </div>

                    <div className="mt-3 flex flex-wrap items-center gap-x-5 gap-y-2 font-comic-relief text-xs text-[#fff0c7] sm:text-sm">
                        <span className="inline-flex items-center gap-2">
                            <span className="size-3 rounded-full border-2 border-[#fffaf0] bg-[#7b3607]" aria-hidden="true" />
                            Input points ({points.length})
                        </span>
                        <span className="inline-flex items-center gap-2">
                            <span className="h-1 w-6 rounded bg-[#73d7ec]" aria-hidden="true" />
                            Least-squares fit
                        </span>
                        <span className="basis-full text-[#fff0c7]/85">
                            On phones, drag the graph to pan and use + / − to zoom. Reset restores the data view.
                        </span>
                    </div>
                </div>
            </WoodPanel>
        </section>
    );

}