from pathlib import Path
import shutil
import subprocess
import sys

from pypdf import PdfWriter

# IMPORTANT PATHS
BASE_DIR = Path(__file__).resolve().parent
INPUT_DIR = BASE_DIR / "../tests/out"
OUTPUT_DIR = INPUT_DIR / "pdf"

INPUT_DIR = INPUT_DIR.resolve()
OUTPUT_DIR = OUTPUT_DIR.resolve()

def compile_latex(source: Path, output_dir: Path) -> Path:
    """Compile LaTeX to PDF"""
    output_pdf = output_dir / f"{source.stem}.pdf"
    
    # Utilizing Tectonic
    print(f"\n[RUNNING] Compiling {source.name}...", flush=True)

    result = subprocess.run(
        [
            "tectonic",
            "--keep-logs",
            "--outdir",
            str(output_dir),
            str(source),
        ],
        stdout=None,
        stderr=None,
        check=False,
    )

    if result.returncode != 0 or not output_pdf.exists():
        raise RuntimeError(
            f"Tectonic failed to compile {source.name} "
            f"(exit code {result.returncode})"
        )

    return output_pdf


def main() -> int:
    if shutil.which("tectonic") is None:
        print("Error: Tectonic is not installed or not in PATH.")
        return 1
    
    if not INPUT_DIR.is_dir():
        print(f"Error: Input directory not found: {INPUT_DIR}")
        return 1
    
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    for pdf in OUTPUT_DIR.glob("*.pdf"):
        pdf.unlink()
    
    sources = sorted(
        (
            path
            for path in INPUT_DIR.glob("*.txt")
            if path.is_file()
        ),
        key=lambda path: path.name.lower(),
    )
    
    if not sources:
        print(f"No .txt files found in {INPUT_DIR}")
        return 0

    successful_pdfs = []
    failed_sources = []

    for source in sources:
        try:
            pdf = compile_latex(source, OUTPUT_DIR)
            successful_pdfs.append(pdf)
            print(f"[OK] {source.name} -> {pdf.name}")

        except (RuntimeError, OSError) as exc:
            failed_sources.append(source.name)
            print(f"[FAIL] {exc}", file=sys.stderr)

    # Merge only PDFs successfully compiled in this run.
    if successful_pdfs:
        combined_pdf = OUTPUT_DIR / "all.pdf"

        try:
            writer = PdfWriter()

            try:
                for pdf in successful_pdfs:
                    writer.append(str(pdf), outline_item=pdf.stem)

                with combined_pdf.open("wb") as file:
                    writer.write(file)
            finally:
                writer.close()

            print(f"\n[OK] Combined PDF: {combined_pdf}")

        except Exception as exc:
            print(
                f"[FAIL] Could not create all.pdf: {exc}",
                file=sys.stderr,
            )
            return 1

    else:
        print("No PDFs were compiled successfully.")
        return 1

    print(
        f"\nFinished: {len(successful_pdfs)}/{len(sources)} "
        "reports compiled."
    )

    if failed_sources:
        print("Failed sources: " + ", ".join(failed_sources))
        return 1

    return 0

main()
