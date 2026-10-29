import { useEffect, useRef } from "react";
import katex from "katex";
import WoodPanel from "./WoodPanel";
import { H1 } from "./Heading";

function stripDisplayDelimiters(latex) {
    return String(latex ?? "")
        .trim()
        .replace(/^\\\[\s*/, "")
        .replace(/\s*\\\]$/, "");
}

export default function LaTeXOutput({ result, isSolving }) {
    const mathRef = useRef(null);

    useEffect(() => {

        if (!mathRef.current) return;

        if (result?.type !== "success" || !result.latex) {
            mathRef.current.innerHTML = "";
            return;
        }

        const expression = stripDisplayDelimiters(result.latex);

        mathRef.current.innerHTML = katex.renderToString(expression, {
            displayMode: true,
            throwOnError: false,
            strict: "ignore",
            trust: false,
            output: "htmlAndMathml",
        });

    }, [result]);

    return (
        <section className="mx-auto mt-7 w-full max-w-5xl" aria-labelledby="linea-output-title">
            <div className="mb-3 flex items-end justify-between gap-3 px-1">
                <div>
                    <p className="font-comic-relief text-xs font-bold uppercase tracking-[0.18em] text-[#185267]">Result</p>
                    <H1 id="linea-output-title" className="text-[#e2ffb8] [text-shadow:0_4px_12px_rgba(0,0,0,0.4)] [-webkit-text-stroke:1px_#0785a8]">
                        Worked solution
                    </H1>
                </div>

                {result?.type === "success" && (
                    <span className="rounded-full border border-[#43845f] bg-[#e5f5df] px-3 py-1 font-comic-relief text-xs font-bold text-[#28583c]">
                        Solved
                    </span>

                )}

                {result?.type === "error" && (
                    <span className="rounded-full border border-[#ae4b3d] bg-[#ffe5dc] px-3 py-1 font-comic-relief text-xs font-bold text-[#852f25]">
                        Needs attention
                    </span>
                )}
            </div>

            <WoodPanel width="100%" className="max-w-5xl" borderColor="#7b3607" textureScale={2}>
                <div className="space-y-3 p-3 sm:p-5">
                    {isSolving ? (
                        <div className="flex min-h-36 items-center justify-center gap-3 rounded-xl bg-[#fff7df] px-4 py-8 text-[#553019]" role="status">
                            <span className="size-5 animate-spin rounded-full border-2 border-[#d9c39b] border-t-[#0785a8]" aria-hidden="true" />
                            <p className="font-comic-relief text-sm sm:text-base">Working through the steps…</p>
                        </div>

                    ) : result?.type === "error" ? (
                        <div className="rounded-xl border-2 border-[#c86b5c] bg-[#fff0e9] p-4 text-[#782b22]" role="alert" aria-live="assertive">
                            <p className="font-gochi-hand text-xl">We couldn’t solve this input yet.</p>
                            <p className="mt-1 wrap-break-word font-comic-relief text-sm leading-relaxed sm:text-base">{result.message}</p>
                            <p className="mt-3 font-comic-relief text-xs leading-relaxed text-[#8c4b3e]">Check the input format, dimensions, and any empty cells, then try again.</p>
                        </div>
                    ) : result?.type === "success" ? (
                        <div
                            ref={mathRef}
                            className="linea-math-output min-w-0 max-w-full overflow-x-auto rounded-xl bg-[#fffaf0] px-3 py-4 text-[#35271b] sm:px-5 sm:py-6"
                            aria-label="Rendered step-by-step mathematical solution"
                        />
                    ) : (
                        <div className="flex min-h-36 flex-col items-center justify-center rounded-xl border border-dashed border-[#d6bd8b] bg-[#fff7df] px-4 py-8 text-center text-[#755b3a]">
                            <span className="font-gochi-hand text-3xl" aria-hidden="true">∑</span>
                            <p className="mt-2 font-gochi-hand text-xl">Your steps will appear here.</p>
                            <p className="mt-1 max-w-md font-comic-relief text-sm leading-relaxed">
                                Enter your numbers or equations above and select <strong>Solve</strong> to get the rendered LaTeX walkthrough.
                            </p>
                        </div>
                    )}
                </div>
            </WoodPanel>
        </section>
    );
}