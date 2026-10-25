const headingStyles = {
    1 : "text-3xl sm:text-4xl lg:text-5xl leading-tight",
    2 : "text-2xl sm:text-3xl lg:text-4xl leading-tight",
    3 : "text-xl sm:text-2xl lg:text-3xl leading-snug",
    4 : "text-lg sm:text-xl lg:text-2xl leading-snug",
    5 : "text-base sm:text-lg lg:text-xl leading-snug",
    6 : "text-sm sm:text-base lg:text-lg leading-snug", 
}

function createHeading(level){
    const Tag = `h${level}`;

    function Heading({ children, className = "", ...props}){
        return(
            <Tag
                className={[
                "font-gochi-hand",
                "font-normal tracking-tight",
                "wrap-break-word",
                headingStyles[level],
                className,
                ].join(" ")}
                {...props}
            >
                {children}
            </Tag>
        )
    }

    Heading.displayName = `H${level}`;
    return Heading;
}

export const H1 = createHeading(1);
export const H2 = createHeading(2);
export const H3 = createHeading(3);