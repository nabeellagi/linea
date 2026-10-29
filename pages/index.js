import { useState } from "react";
import CloudBackground from "@/components/CloudBackground";
import { H2 } from "@/components/Heading";
import { T1 } from "@/components/Title";
import LineaWorkspace from "@/components/LineaWorkspace";

export default function Home() {
  const [mode, setMode] = useState("determinant");

  return (

    <main className="relative isolate h-screen overflow-hidden">
      <title>LINEA! ALL THINGS MATRIX</title>
      <div className="pointer-events-none fixed inset-0 -z-10">
        <CloudBackground
          maxClouds={12}
          minSpeed={8}
          maxSpeed={35}
          opacity={0.8}
        />
      </div>

      <div className="relative z-10 h-full overflow-y-auto snap-y snap-mandatory scroll-smooth">
        <section className="flex min-h-screen snap-start snap-always items-center justify-center px-4 py-16">
          <div className="w-full max-w-5xl text-center">
            <T1 className="text-[#e2ffb8] [text-shadow:0_4px_12px_rgba(0,0,0,0.4)] [-webkit-text-stroke:2px_#0785a8]">
              LINEA
            </T1>

            <H2 className="mt-2 text-[#0785a8] [text-shadow:0_4px_12px_rgba(0,0,0,0.4)]">
              Anything matrix. Solve it here!
            </H2>

            <p className="mt-5 font-comic-relief text-base text-[#184c5a] sm:text-lg">
              Determinants, systems of equations, and regression, one clear step at a time.
            </p>

            <button
              type="button"
              onClick={() => {
                document.getElementById("linea-workspace")?.scrollIntoView({
                  behavior: "smooth",
                  block: "start",
                });

              }}
              className="mt-8 rounded-full border-2 border-[#7b3607] bg-[#fff0c7] px-6 py-3 font-gochi-hand text-lg text-[#553019] shadow-[0_4px_0_#7b3607] transition hover:-translate-y-0.5 hover:bg-[#fff7e4] active:translate-y-0 active:shadow-none"
            >
              Open the worktable ↓
            </button>
          </div>
        </section>

        <section
          id="linea-workspace"
          className="min-h-screen snap-start snap-always px-3 py-10 sm:px-6 sm:py-14"
        >
          <LineaWorkspace mode={mode} onModeChange={setMode} />
          <p className="mx-auto mt-8 max-w-4xl pb-4 text-center font-comic-relief text-xs text-[#185267] sm:text-sm">
            LINEA keeps the calculation in WebAssembly and renders the returned steps as LaTeX.
          </p>
        </section>
      </div>
    </main>
  );
}