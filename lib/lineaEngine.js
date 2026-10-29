let enginePromise = null;

function loadEngineScript() {
    if (typeof window === "undefined") {
        return Promise.reject(new Error("The LINEA WebAssembly engine can only be loaded in the browser."));
    }

    if (typeof window.createMat === "function") return Promise.resolve();

    return new Promise((resolve, reject) => {
        const existingScript = document.querySelector("script[data-linea-engine='true']");

        if (existingScript?.dataset.loaded === "true") {
            reject(new Error("The WebAssembly loader script loaded, but its createMat factory was not exposed. Rebuild wasm/build/mat.js and check the Emscripten flags."));
            return;
        }

        const script = existingScript ?? document.createElement("script");

        const onLoad = () => {
            script.dataset.loaded = "true";
            if (typeof window.createMat === "function") resolve();
            else reject(new Error("The WebAssembly loader did not expose createMat. Check wasm/build.bat and rebuild the engine."));
        };

        const onError = () => {
            script.remove();
            reject(new Error("Could not load /wasm/mat.js. Build the WebAssembly engine and copy mat.js and mat.wasm into public/wasm/."));
        };

        script.addEventListener("load", onLoad, { once: true });
        script.addEventListener("error", onError, { once: true });

        if (!existingScript) {
            script.src = "/wasm/mat.js";
            script.async = true;
            script.dataset.lineaEngine = "true";
            document.head.appendChild(script);
        }
    });
}

async function createEngine() {
    await loadEngineScript();

    const module = await window.createMat({
        locateFile: (path) => path.endsWith(".wasm") ? "/wasm/mat.wasm" : path,
    });

    return {
        det: module.cwrap("det", "string", ["string"]),
        detStatus: module.cwrap("det_status", "number", []),
        solveGauss: module.cwrap("solve_gauss", "string", ["string"]),
        solveGaussStatus: module.cwrap("solve_gauss_status", "number", []),
        solveLu: module.cwrap("solve_lu", "string", ["string"]),
        solveLuStatus: module.cwrap("solve_lu_status", "number", []),
        regression: module.cwrap("linear_regression", "string", ["string"]),
        regressionStatus: module.cwrap("linear_regression_status", "number", []),
    };
}

function getEngine() {
    if (!enginePromise) {
        enginePromise = createEngine().catch((error) => {
            enginePromise = null;
            throw error;
        });
    }
    return enginePromise;
}

function normalizeOutcome(text, status) {
    const output = String(text ?? "").trim();
    if (status === 0) return { ok: true, latex: output, error: null, status };
    return {
        ok: false,
        latex: "",
        error: output || `The calculation failed with engine status ${status}.`,
        status,
    };
}

export async function runDeterminant(matrixText) {
    const engine = await getEngine();
    const output = engine.det(matrixText);
    return normalizeOutcome(output, engine.detStatus());
}

export async function runSystemSolver(equations, method = "gauss") {
    const engine = await getEngine();
    const useLu = method === "lu";
    const output = useLu ? engine.solveLu(equations) : engine.solveGauss(equations);
    const status = useLu ? engine.solveLuStatus() : engine.solveGaussStatus();
    return normalizeOutcome(output, status);

}

export async function runRegression(points) {
    const engine = await getEngine();
    const output = engine.regression(points);
    return normalizeOutcome(output, engine.regressionStatus());
}