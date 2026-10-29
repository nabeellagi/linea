const MIN_SIZE = 1;
const MAX_SIZE = 12;


function resizeSquareMatrix(matrix, nextSize) {
    return Array.from({ length: nextSize }, (_, row) =>
        Array.from({ length: nextSize }, (_, col) => matrix[row]?.[col] ?? "")
    );
}

export default function DeterminantGrid({ size, setSize, matrix, setMatrix, onEdit }) {
    function changeSize(nextSize) {
        const clamped = Math.max(MIN_SIZE, Math.min(MAX_SIZE, nextSize));
        setMatrix((current) => resizeSquareMatrix(current, clamped));
        setSize(clamped);
        onEdit?.();
    }

    function updateCell(rowIndex, colIndex, value) {
        setMatrix((current) =>
            current.map((row, r) =>
                r === rowIndex
                    ? row.map((cell, c) => (c === colIndex ? value : cell))
                    : row
            )
        );
        onEdit?.();
    }

    function clearMatrix() {
        setMatrix(Array.from({ length: size }, () => Array(size).fill("")));
        onEdit?.();
    }

    function loadExample() {
        const example = [
            ["1", "2", "3"],
            ["0", "1", "4"],
            ["5", "6", "0"]
        ];
        setSize(3);
        setMatrix(example);
        onEdit?.();
    }

    const minimumGridWidth = Math.max(100, size * 48);

    return (
        <div className="space-y-4">
            <div className="flex flex-wrap items-center justify-between gap-3">
                <div>
                    <p className="font-gochi-hand text-lg text-[#fff0c7] sm:text-xl">Square matrix</p>
                    <p className="font-comic-relief text-xs text-[#fff0c7]/90 sm:text-sm">
                        {size} × {size} · numbers only
                    </p>
                </div>
                <div className="flex items-center gap-2 rounded-xl border-2 border-[#7b3607] bg-[#fff0c7] p-1.5 text-[#553019]">
                    <button
                        type="button"
                        onClick={() => changeSize(size - 1)}
                        disabled={size <= MIN_SIZE}
                        aria-label="Decrease matrix size"
                        className="grid size-9 place-items-center rounded-lg font-gochi-hand text-2xl leading-none hover:bg-[#ead4a3] disabled:cursor-not-allowed disabled:opacity-35"

                    >
                        −
                    </button>

                    <span className="min-w-12 text-center font-comic-relief text-sm font-bold" aria-live="polite">
                        {size} × {size}
                    </span>

                    <button
                        type="button"
                        onClick={() => changeSize(size + 1)}
                        disabled={size >= MAX_SIZE}
                        aria-label="Increase matrix size"
                        className="grid size-9 place-items-center rounded-lg font-gochi-hand text-2xl leading-none hover:bg-[#ead4a3] disabled:cursor-not-allowed disabled:opacity-35"
                    >
                        +
                    </button>
                </div>
            </div>

            <div className="rounded-xl border-2 border-[#7b3607] bg-[#fff7df] p-2.5 sm:p-4">
                <div className="overflow-x-auto pb-1">
                    <div
                        className="mx-auto grid gap-1.5 sm:gap-2"
                        style={{
                            gridTemplateColumns: `repeat(${size}, minmax(2.4rem, 1fr))`,
                            minWidth: `${minimumGridWidth}px`,
                            maxWidth: size <= 4 ? "34rem" : undefined,
                        }}
                    >
                        {matrix.map((row, rowIndex) =>
                            row.map((value, colIndex) => (
                                <input
                                    key={`${rowIndex}-${colIndex}`}
                                    type="text"
                                    inputMode="decimal"
                                    autoComplete="off"
                                    spellCheck={false}
                                    aria-label={`Row ${rowIndex + 1}, column ${colIndex + 1}`}
                                    value={value}
                                    onChange={(event) => updateCell(rowIndex, colIndex, event.target.value)}
                                    placeholder="0"
                                    className="h-11 w-full min-w-0 rounded-lg border border-[#d6bd8b] bg-white px-1 text-center font-mono text-sm text-[#3d2b1b] outline-none transition placeholder:text-[#bda783] focus:border-[#0785a8] focus:ring-2 focus:ring-[#0785a8]/25 sm:h-12 sm:text-base"
                                />
                            ))
                        )}
                    </div>
                </div>

                <div className="mt-3 flex flex-wrap justify-end gap-2">
                    <button
                        type="button"
                        onClick={clearMatrix}
                        className="rounded-full border border-[#b99b68] px-3 py-1.5 font-comic-relief text-xs text-[#60472d] transition hover:bg-[#f4e6c4] sm:text-sm"
                    >
                        Clear matrix
                    </button>

                    <button
                        type="button"
                        onClick={loadExample}
                        className="rounded-full border border-[#b99b68] px-3 py-1.5 font-comic-relief text-xs text-[#60472d] transition hover:bg-[#f4e6c4] sm:text-sm"

                    >
                        Load example
                    </button>
                </div>
            </div>

            <p className="font-comic-relief text-xs leading-relaxed text-[#fff0c7]/90 sm:text-sm">
                LUP is used for the determinant. For sizes up to 3 × 3, LINEA also shows minor/cofactor expansion as a cross-check.
            </p>
        </div>
    );
}

