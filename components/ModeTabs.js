
const MODES = [
    { id: "determinant", label: "Determinant" },
    { id: "system", label: "System Solver" },
    { id: "regression", label: "Linear Regression" },
];

export default function ModeTabs({
    mode,
    onChange,
    disabled = false,
}) {
    return (
        <div
            className="relative z-20 mx-auto -mb-0.75 grid w-full max-w-5xl grid-cols-3 gap-1 px-1 sm:px-3"
            role="tablist"
            aria-label="Choose a matrix tool"
        >
            {MODES.map((item) => {
                const active = mode === item.id;

                return (
                    <button
                        key={item.id}
                        id={`linea-tab-${item.id}`}
                        type="button"
                        disabled={disabled}
                        role="tab"
                        aria-selected={active}
                        aria-controls="linea-input-panel"
                        onClick={() => onChange(item.id)}
                        className={[
                            // Layout: keep each tab inside its grid column.
                            "flex min-h-12 min-w-0 w-full",
                            "items-center justify-center",
                            "rounded-t-xl border-2 border-b-0",
                            "border-[#7b3607]",

                            // Phone typography and spacing.
                            "px-1 py-2 text-[11px] leading-tight",

                            // Desktop typography and spacing.
                            "sm:min-h-14 sm:px-3 sm:py-3 sm:text-lg",

                            // Keep the LINEA visual style.
                            "text-center font-gochi-hand transition",

                            active
                                ? "bg-[#fff0c7] text-[#553019] shadow-[0_-3px_0_rgba(123,54,7,0.12)]"
                                : "bg-[#ead4a3] text-[#725239] hover:bg-[#f8e7bd]",

                            disabled
                                ? "cursor-wait opacity-70"
                                : "",
                        ].join(" ")}
                    >
                        <span className="block w-full whitespace-normal wrap-break-word text-center">
                            {item.label}
                        </span>
                    </button>
                );
            })}
        </div>
    );
}
