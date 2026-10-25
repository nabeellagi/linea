const titleStyles = {
    1 : "text-7xl sm:text-8xl lg:text-9xl leading-[1.12] tracking-wide",
    2 : "text-4xl sm:text-5xl lg:text-7xl leading-[1.12] tracking-wide",
    3 : "text-3xl sm:text-4xl lg:text-5xl leading-[1.15] tracking-wide",
    4: "text-2xl sm:text-3xl lg:text-4xl leading-[1.2] tracking-wide",
}

function createTitle(level){
    const Tag = `h${level}`;

    function Title({ children, className = "", ...props }){
        return (
            <Tag
                className = {[
                    "font-agbalumo",
                    "font-normal tracking-tight",
                    "text-balance wrap-break-word",
                    titleStyles[level],
                    className,
                ].join(" ")}
                {...props}
            >
                {children}
            </Tag>
        )
    }

    Title.displayName = `T${level}`;
    return Title;
}

export const T1 = createTitle(1);
export const T2 = createTitle(2);
export const T3 = createTitle(3);