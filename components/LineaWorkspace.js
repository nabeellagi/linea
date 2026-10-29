import { useMemo, useState } from "react";
import { H2 } from "@/components/Heading";
import WoodPanel from "@/components/WoodPanel";
import ModeTabs from "@/components/ModeTabs";
import InputGuidePopover from "@/components/InputGuidePopover";
import DeterminantGrid from "@/components/DeterminantGrid";
import LaTeXOutput from "@/components/LaTeXOutput";
import { runDeterminant, runRegression, runSystemSolver } from "@/lib/lineaEngine";
import P from "./Paragraph";
import LinearRegressionGraph from "./LinearRegressionGraph";

const START_MATRIX = [
    ["0", "1", "2"],
    ["0", "1", "4"],
    ["5", "6", "0"]
]

const MODE_TITLES = {
    determinant: "Determinant",
    system: "System Solver",
    regression: "Linear Regression",
};

function inputFontSize(length) {
    if (length > 1400) return "12px";
    if (length > 900) return "13px";
    if (length > 500) return "14px";
    if (length > 250) return "15px";
    if (length > 120) return "16px";
    return "18px";
}


export default function LineaWorkspace({ mode, onModeChange }) {
    const [matrixSize, setMatrixSize] = useState(3);
    const [matrix, setMatrix] = useState(START_MATRIX);
    const [systemMethod, setSystemMethod] = useState("gauss");
    const [systemInput, setSystemInput] = useState("x + y = 5\n2x - y = 1");
    const [regressionInput, setRegressionInput] = useState("(0, 1) (1, 2) (2, 2.5)");
    const [result, setResult] = useState(null);
    const [isSolving, setIsSolving] = useState(false);

    const activeText = mode === "system" ? systemInput : regressionInput;
    const activeFontSize = useMemo(() => inputFontSize(activeText.length), [activeText.length]);

    const switchMode = (nextMode) => {
        onModeChange(nextMode)
        // setResult(null)
    }

    const matrixIsComplete = () => {
        return matrix.every((row) => row.length === matrixSize && row.every((cell) => cell.trim() !== ""));
    }

    async function handleSolve(event) {
        event.preventDefault();
        setIsSolving(true)
        setResult(null);

        let outcome;
        try {
            if (mode === "determinant") {
                if (!matrixIsComplete()) {
                    setResult({ type: "error", message: "Fill every matrix cell before solving. Use the Clear matrix or Load example controls if needed." });
                    return;
                }

                const serializedMatrix = matrix.map((row) => row.map((cell) => cell.trim()).join(" ")).join("\n");
                outcome = await runDeterminant(serializedMatrix);
            } else if (mode === "system") {
                if (!systemInput.trim()) {
                    setResult({ type: "error", message: "Enter one equation per line before solving." });
                    return;
                }
                outcome = await runSystemSolver(systemInput, systemMethod);
            } else {
                if (!regressionInput.trim()) {
                    setResult({ type: "error", message: "Enter at least two coordinate pairs before solving." });
                    return;
                }
                outcome = await runRegression(regressionInput);
            }

            if (outcome.ok) {
                setResult({ type: "success", latex: outcome.latex });
            } else {
                setResult({ type: "error", message: outcome.error, status: outcome.status });
            }
        } catch (error) {
            setResult({
                type: "error",
                message: error instanceof Error
                    ? error.message
                    : "LINEA could not start its WebAssembly engine. Check that the WASM files have been built and copied to public/wasm/.",
            });
        } finally {
            setIsSolving(false);
        }
    }

    return (
        <div className="mx-auto w-full max-w-5xl">
            <ModeTabs mode={mode} onChange={switchMode} disabled={isSolving} />

            <div className="relative w-full">
                <div className="absolute right-4 top-4 z-40 sm:right-6 sm:top-6">
                    <InputGuidePopover mode={mode} />
                </div>
                <WoodPanel width="100%" className="max-w-5xl" borderColor="#7b3607" textureScale={2}>
                    <form onSubmit={handleSolve}>
                        <div
                            id="linea-input-panel"
                            role="tabpanel"
                            aria-labelledby={`linea-tab-${mode}`}
                            className="p-4 sm:p-6 md:p-8"
                        >
                            <div className="mb-6 flex items-start gap-3 pr-10 sm:mb-7 sm:pr-12">
                                <div className="min-w-0">
                                    <P className="font-comic-relief text-xs font-bold uppercase tracking-[0.18em] text-[#fff0c7]/85 sm:text-sm">
                                        LINEA worktable
                                    </P>
                                    <H2 className="mt-1 text-[#fff0c7] [text-shadow:0_2px_5px_rgba(0,0,0,0.35)]">
                                        {MODE_TITLES[mode]}
                                    </H2>
                                </div>
                            </div>

                            {mode === "determinant" ? (
                                <DeterminantGrid
                                    size={matrixSize}
                                    setSize={setMatrixSize}
                                    matrix={matrix}
                                    setMatrix={setMatrix}
                                    onEdit={() => setResult(null)}
                                />
                            ) : (
                                <div className="space-y-4">
                                    {mode === "system" ? (
                                        <>
                                            <div>
                                                <P className="font-gochi-hand text-lg text-[#fff0c7] sm:text-xl">Your equations</P>
                                                <P className="mt-1 font-comic-relief text-xs leading-relaxed text-[#fff0c7]/90 sm:text-sm">
                                                    One equation per line. Example: <code className="rounded bg-black/15 px-1">x + y = 5</code>
                                                </P>
                                            </div>

                                            <div className="flex flex-wrap items-center gap-2" aria-label="System solving method">
                                                <span className="mr-1 font-comic-relief text-sm text-[#fff0c7]">Method</span>
                                                <button
                                                    type="button"
                                                    onClick={() => { setSystemMethod("gauss"); setResult(null); }}
                                                    aria-pressed={systemMethod === "gauss"}
                                                    className={`rounded-full border-2 px-4 py-2 font-gochi-hand text-base transition sm:text-lg ${systemMethod === "gauss" ? "border-[#fff0c7] bg-[#fff0c7] text-[#553019]" : "border-[#fff0c7]/60 bg-black/10 text-[#fff0c7] hover:bg-black/20"}`}
                                                >
                                                    Gaussian
                                                </button>
                                                <button
                                                    type="button"
                                                    onClick={() => { setSystemMethod("lu"); setResult(null); }}
                                                    aria-pressed={systemMethod === "lu"}
                                                    className={`rounded-full border-2 px-4 py-2 font-gochi-hand text-base transition sm:text-lg ${systemMethod === "lu" ? "border-[#fff0c7] bg-[#fff0c7] text-[#553019]" : "border-[#fff0c7]/60 bg-black/10 text-[#fff0c7] hover:bg-black/20"}`}
                                                >
                                                    LU decomposition
                                                </button>
                                            </div>
                                        </>
                                    ) : (
                                        <div>
                                            <P className="font-gochi-hand text-lg text-[#fff0c7] sm:text-xl">Observed coordinate points</P>
                                            <P className="mt-1 font-comic-relief text-xs leading-relaxed text-[#fff0c7]/90 sm:text-sm">
                                                Enter points as <code className="rounded bg-black/15 px-1">(x, y)</code>, separated by spaces or new lines.
                                            </P>
                                        </div>
                                    )}

                                    <label className="block">
                                        <span className="sr-only">
                                            {mode === "system" ? "System of linear equations" : "Regression coordinate pairs"}
                                        </span>
                                        <textarea
                                            value={mode === "system" ? systemInput : regressionInput}
                                            onChange={(event) => {
                                                if (mode === "system") setSystemInput(event.target.value);
                                                else setRegressionInput(event.target.value);
                                                setResult(null);
                                            }}
                                            spellCheck={false}
                                            autoCapitalize="off"
                                            autoCorrect="off"
                                            autoComplete="off"
                                            rows={7}
                                            placeholder={mode === "system" ? "x + y = 5\n2x - y = 1" : "(0, 1) (1, 2) (2, 2.5)"}
                                            style={{ fontSize: activeFontSize }}
                                            className="linea-input-area block min-h-56 max-h-[48vh] w-full resize-y rounded-xl border-2 border-[#7b3607] bg-[#fff7df] p-4 font-mono leading-relaxed text-[#3d2b1b] shadow-inner outline-none transition placeholder:text-[#bba786] focus:border-[#0785a8] focus:ring-4 focus:ring-[#0785a8]/20 sm:min-h-72 sm:p-5"
                                        />
                                    </label>
                                    <div className="flex items-center justify-between gap-3 font-comic-relief text-xs text-[#fff0c7]/80 sm:text-sm">
                                        <span>{activeText.length.toLocaleString()} characters</span>
                                        <button
                                            type="button"
                                            onClick={() => {
                                                if (mode === "system") setSystemInput("");
                                                else setRegressionInput("");
                                                setResult(null);
                                            }}
                                            className="rounded-full px-3 py-1.5 underline decoration-[#fff0c7]/50 underline-offset-2 transition hover:bg-black/10"
                                        >
                                            Clear input
                                        </button>
                                    </div>
                                </div>
                            )}

                            <div className="mt-6 flex flex-col-reverse gap-3 border-t border-[#fff0c7]/25 pt-5 sm:mt-7 sm:flex-row sm:items-center sm:justify-between">
                                <p className="font-comic-relief text-xs leading-relaxed text-[#fff0c7]/85 sm:max-w-[60%] sm:text-sm">
                                    {mode === "determinant"
                                        ? "Enter a square matrix. The current grid supports sizes from 1×1 to 12×12."
                                        : mode === "system"
                                            ? "The current solver expects a square system with one unique solution."
                                            : "Least-squares linear regression uses the coordinate pairs you provide."}
                                </p>
                                <button
                                    type="submit"
                                    disabled={isSolving}
                                    className="inline-flex min-h-12 shrink-0 items-center justify-center gap-2 rounded-xl border-2 border-[#7b3607] bg-[#fff0c7] px-7 py-3 font-gochi-hand text-xl text-[#553019] shadow-[0_4px_0_#7b3607] transition hover:-translate-y-0.5 hover:bg-white active:translate-y-0 active:shadow-none disabled:cursor-wait disabled:opacity-70 sm:min-w-40"
                                >
                                    {isSolving ? "Solving…" : "Solve →"}
                                </button>
                            </div>
                        </div>
                    </form>
                </WoodPanel>
            </div>
            <LaTeXOutput result={result} isSolving={isSolving} />
            {mode === "regression" && result?.type === "success" && (
                <LinearRegressionGraph input={regressionInput} />
            )}
        </div>
    )
}