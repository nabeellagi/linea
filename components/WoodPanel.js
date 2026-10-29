
export default function WoodPanel({
    children,
    className = "",
    borderColor = "#7b3607",
    width = "85%",
    height,
    textureScale = 1,
    style,
}) {
    const textureSize = 64 * textureScale;

    return (
        <div
            className={`
                box-border
                mx-auto
                rounded-xl
                border-4
                overflow-hidden
                text-[#e2ffb8]
                ${className}
            `}
            style={{
                width,
                ...(height ? { height } : {}),
                borderColor,
                backgroundImage: "url('/wood.png')",
                backgroundRepeat: "repeat",
                backgroundSize: `${textureSize}px ${textureSize}px`,
                imageRendering: "pixelated",
                ...style,
            }}
        >
            {children}
        </div>
    );
}
