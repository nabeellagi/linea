import { useEffect, useRef, useState } from "react";

const GUIDE = {
    determinant: {
        title: "Matrix input guide",
        intro: "Fill every cell in the square grid. Integers, negative numbers, and finite decimals are accepted.",
        example: "1   2   3\n0   1   4\n5   6   0",
        note: "The engine uses LUP decomposition. For matrices up to 3×3, it also shows a minor/cofactor expansion and checks that both answers agree. For larger matrices, it uses LUP.",
    },
    system: {
        title: "Equation input guide",
        intro: "Write one equation per line. Use variable names consistently, and make sure the number of equations equals the number of distinct variables for a unique square-system solve.",
        example: "x + y = 5\n2x - y = 1",
        note: "Use + or - between terms. Coefficients can touch a variable (2x) or use * (2*x). Use decimal numbers like 2.5, not fractions like 1/2.",
    },
    regression: {
        title: "Coordinate input guide",
        intro: "Enter at least two coordinate pairs. Each pair must be written as (x, y). Separate pairs with spaces or new lines.",
        example: "(0, 1) (1, 2)\n(2, 2.5)",
        note: "Use integers or finite decimals. The regression engine accepts up to 100 coordinate pairs.",
    },
};

export default function InputGuideProvider({ mode }) {
    const [open, setOpen] = useState(false);
    const rootRef = useRef(null);
    const guide = GUIDE[mode] ?? GUIDE.determinant;

    useEffect(() => {
        if (!open) return undefined;

        function closeOnOutside(event) {
            if (rootRef.current && !rootRef.current.contains(event.target)) {
                setOpen(false);
            }
        }
        function closeOnEscape(event) {
            if (event.key === "Escape") setOpen(false);
        }

        document.addEventListener("pointerdown", closeOnOutside);
        document.addEventListener("keydown", closeOnEscape);
        return () => {
            document.removeEventListener("pointerdown", closeOnOutside);
            document.removeEventListener("keydown", closeOnEscape);
        };
    }, [open])

    return (

        <div className="relative inline-flex shrink-0" ref={rootRef}>
            <button
                type="button"
                aria-label="Open input format guide"
                aria-expanded={open}
                aria-haspopup="dialog"
                onClick={() => setOpen((value) => !value)}
                className="grid size-8 place-items-center rounded-full border-2 border-[#7b3607] bg-[#fff0c7] font-gochi-hand text-xl font-bold leading-none text-[#5b351c] shadow-sm transition hover:rotate-3 hover:bg-white sm:size-9"

            >
                !
            </button>

            {open && (
                <div
                    role="dialog"
                    aria-label={guide.title}
                    className="absolute right-0 top-[calc(100%+0.6rem)] z-50 w-[min(88vw,23rem)] rounded-xl border-2 border-[#7b3607] bg-[#fff7df] p-4 text-left text-[#3d2b1b] shadow-[0_12px_30px_rgba(57,32,10,0.25)]"
                >
                    <div className="flex items-start justify-between gap-3">
                        <h3 className="font-gochi-hand text-xl leading-tight">{guide.title}</h3>
                        <button
                            type="button"
                            aria-label="Close input guide"
                            onClick={() => setOpen(false)}
                            className="grid size-7 shrink-0 place-items-center rounded-full bg-[#ead4a3] font-bold hover:bg-[#e1c287]"

                        >
                            ×
                        </button>
                    </div>

                    <p className="mt-2 font-comic-relief text-sm leading-relaxed">{guide.intro}</p>
                    <p className="mt-3 font-gochi-hand text-base">Example</p>
                    <pre className="mt-1 overflow-x-auto rounded-lg border border-[#ddc79b] bg-white/80 p-3 font-mono text-xs leading-relaxed text-[#342519] sm:text-sm">
                        {guide.example}
                    </pre>

                    <p className="mt-3 font-comic-relief text-xs leading-relaxed text-[#674c31]">{guide.note}</p>
                </div>
            )}
        </div>

    );
}