function Class(base)
    local cls = {} 

    if base then
        setmetatable(cls, { __index = base })
        cls.super = base
    end

    function cls:new(...)
        local instance = setmetatable({}, { __index = cls }) 
        if instance.constructor then
            instance:constructor(...)
        end
        return instance
    end

    return cls
end
 