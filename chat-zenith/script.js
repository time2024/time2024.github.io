document.addEventListener('DOMContentLoaded', function() {
    // 配置marked.js - 用于渲染Markdown格式的回复
    marked.setOptions({
        highlight: function(code, lang) {
            const language = hljs.getLanguage(lang) ? lang : 'plaintext';
            return hljs.highlight(code, { language }).value;
        },
        breaks: true,
        gfm: true
    });

    const chatHistory = document.getElementById('chat-history');
    const userInput = document.getElementById('user-input');
    const sendBtn = document.getElementById('send-btn');
    const clearBtn = document.getElementById('clear-btn');
    
    // API端点配置
    // 使用HTTPS协议访问你配置的子域名
    const API_URL = 'https://api.smallgoodgood.top/chat';
    
    // 对话历史记录，用于保持上下文
    let conversationHistory = [];
    
    /**
     * 发送消息的主函数
     * 处理用户输入，调用API，显示响应
     */
    async function sendMessage() {
        const message = userInput.value.trim();
        if (!message) return;
        
        // 禁用输入控件，防止重复发送
        sendBtn.disabled = true;
        userInput.disabled = true;
        
        // 将用户消息添加到聊天界面
        addMessageToChat('user', message);
        userInput.value = '';
        
        // 显示AI正在思考的提示
        const thinkingMsgId = showThinkingMessage();
        
        try {
            // 调用API获取响应
            console.log('正在发送请求到:', API_URL);
            const response = await callLangchainAPI(message);
            console.log('收到响应:', response);
            
            // 移除思考提示
            removeThinkingMessage(thinkingMsgId);
            
            // 处理响应 - 检查响应格式
            if (response && response.choices && response.choices[0]) {
                const aiMessage = response.choices[0].message;
                if (aiMessage && aiMessage.content) {
                    const aiResponse = aiMessage.content;
                    addMessageToChat('assistant', aiResponse, true); // 标记为需要渲染Markdown
                    
                    // 更新对话历史
                    conversationHistory.push(
                        { role: 'user', content: message },
                        { role: 'assistant', content: aiResponse }
                    );
                } else {
                    // 响应格式不完整
                    console.error('响应格式不完整，message对象:', aiMessage);
                    throw new Error('API响应格式不正确：缺少message.content');
                }
            } else {
                // 响应格式完全错误
                console.error('响应格式错误，完整响应:', response);
                throw new Error('API响应格式不正确：缺少choices数组');
            }
        } catch (error) {
            removeThinkingMessage(thinkingMsgId);
            console.error('处理消息时出错:', error);
            
            // 提供详细的错误信息
            let errorMessage = '抱歉，出现错误: ';
            
            // 根据错误类型提供更具体的错误信息
            if (error.name === 'TypeError' && error.message.includes('Failed to fetch')) {
                errorMessage += '无法连接到服务器，请检查网络连接';
            } else if (error.message.includes('JSON')) {
                errorMessage += '服务器响应格式错误';
            } else if (error.message.includes('API响应格式')) {
                errorMessage += error.message;
            } else {
                errorMessage += error.message || '未知错误';
            }
            
            addMessageToChat('assistant', errorMessage);
        } finally {
            // 恢复输入控件
            sendBtn.disabled = false;
            userInput.disabled = false;
            userInput.focus();
        }
    }
    
    /**
     * 调用Langchain API的函数
     * @param {string} message - 用户输入的消息
     * @returns {Promise} API响应
     */
    async function callLangchainAPI(message) {
        try {
            // 构建请求体
            const requestBody = {
                messages: [
                    ...conversationHistory,    // 包含历史对话
                    { role: 'user', content: message }
                ]
            };
            
            console.log('发送请求体:', requestBody);
            
            // 发送POST请求到API端点
            const response = await fetch(API_URL, {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify(requestBody)
            });
            
            console.log('响应状态:', response.status, response.statusText);
            console.log('响应头:', response.headers);
            
            // 检查响应状态
            if (!response.ok) {
                let errorMessage = `请求失败: ${response.status} ${response.statusText}`;
                try {
                    const errorData = await response.json();
                    if (errorData.error) {
                        errorMessage = errorData.error;
                    }
                } catch (e) {
                    // 如果无法解析错误响应，使用默认错误信息
                    console.error('无法解析错误响应:', e);
                }
                throw new Error(errorMessage);
            }
            
            // 尝试解析JSON响应
            let data;
            try {
                const responseText = await response.text();
                console.log('原始响应文本:', responseText);
                
                // 尝试解析JSON
                data = JSON.parse(responseText);
                console.log('解析后的JSON:', data);
            } catch (parseError) {
                console.error('JSON解析失败:', parseError);
                throw new Error('服务器响应格式错误，无法解析JSON');
            }
            
            return data;
            
        } catch (error) {
            console.error('API调用失败:', error);
            
            // 重新抛出错误，让上层处理
            throw error;
        }
    }
    
    /**
     * 添加消息到聊天界面
     * 支持Markdown渲染，AI消息统一样式
     */
    function addMessageToChat(role, content, isMarkdown = false) {
        const messageDiv = document.createElement('div');
        
        if (role === 'assistant') {
            // AI消息样式
            messageDiv.className = 'message ai-message';
            messageDiv.style.display = 'flex';
            messageDiv.style.alignItems = 'flex-start';

            // 添加猫咪头像
            const avatar = document.createElement('img');
            avatar.src = 'https://smallgoodgood.top/images/23.jpg';
            avatar.className = 'cat-avatar';
            messageDiv.appendChild(avatar);

            // 消息内容容器
            const contentDiv = document.createElement('div');
            contentDiv.style.flex = '1';
            
            if (isMarkdown) {
                // 处理数学公式并渲染Markdown
                let mathContent = content
                    .replace(/\\\((.+?)\\\)/g, function(match, p1) {
                        return '$' + p1 + '$';
                    })
                    .replace(/\\\[(.+?)\\\]/g, function(match, p1) {
                        return '$$' + p1 + '$$';
                    });
                contentDiv.innerHTML = marked.parse(mathContent);
            } else {
                contentDiv.textContent = content;
            }
            messageDiv.appendChild(contentDiv);
        } else {
            // 用户消息样式
            messageDiv.className = 'message user-message';
            const messageContent = document.createElement('div');
            messageContent.textContent = content;
            messageDiv.appendChild(messageContent);
        }

        // 添加时间戳
        const messageTime = document.createElement('div');
        messageTime.className = 'message-time';
        messageTime.textContent = new Date().toLocaleTimeString();
        messageDiv.appendChild(messageTime);

        // 添加到聊天历史
        chatHistory.appendChild(messageDiv);
        chatHistory.scrollTop = chatHistory.scrollHeight;

        // 高亮代码块（如果有）
        if (isMarkdown && role === 'assistant') {
            messageDiv.querySelectorAll('pre code').forEach((block) => {
                hljs.highlightElement(block);
            });
        }

        // 渲染数学公式（如果MathJax可用）
        if (window.MathJax && typeof MathJax.typesetPromise === 'function') {
            MathJax.typesetPromise([messageDiv]).catch(err => {
                console.error('MathJax渲染错误:', err);
            });
        }
    }

    /**
     * 显示AI思考中的动画提示
     */
    function showThinkingMessage() {
        const messageDiv = document.createElement('div');
        messageDiv.className = 'message ai-message';
        messageDiv.id = 'thinking-message';

        const innerDiv = document.createElement('div');
        innerDiv.style.display = 'flex';
        innerDiv.style.alignItems = 'flex-start';

        const avatar = document.createElement('img');
        avatar.src = 'https://smallgoodgood.top/images/23.jpg';
        avatar.className = 'cat-avatar';
        innerDiv.appendChild(avatar);

        const contentDiv = document.createElement('div');
        const thinkingText = document.createElement('span');
        thinkingText.textContent = '正在思考...';
        contentDiv.appendChild(thinkingText);
        
        const loadingSpan = document.createElement('span');
        loadingSpan.className = 'loading';
        contentDiv.appendChild(loadingSpan);
        innerDiv.appendChild(contentDiv);

        messageDiv.appendChild(innerDiv);

        const messageTime = document.createElement('div');
        messageTime.className = 'message-time';
        messageTime.textContent = new Date().toLocaleTimeString();
        messageDiv.appendChild(messageTime);

        chatHistory.appendChild(messageDiv);
        chatHistory.scrollTop = chatHistory.scrollHeight;

        return 'thinking-message';
    }
    
    /**
     * 移除思考提示
     */
    function removeThinkingMessage(messageId) {
        const thinkingMsg = document.getElementById(messageId);
        if (thinkingMsg) {
            thinkingMsg.remove();
        }
    }
    
    /**
     * 清空聊天历史
     */
    function clearChatHistory() {
        // 保留欢迎消息（如果有）
        const welcomeMessage = chatHistory.querySelector('.ai-message');
        chatHistory.innerHTML = '';
        if (welcomeMessage) {
            chatHistory.appendChild(welcomeMessage.cloneNode(true));
        }
        conversationHistory = [];
        console.log('聊天历史已清空');
    }
    
    // ===== 事件监听器 =====
    sendBtn.addEventListener('click', sendMessage);
    
    // Enter键发送消息（Shift+Enter换行）
    userInput.addEventListener('keydown', function(e) {
        if (e.key === 'Enter' && !e.shiftKey) {
            e.preventDefault();
            sendMessage();
        }
    });
    
    // 清空按钮
    clearBtn.addEventListener('click', clearChatHistory);
    
    // 限制输入字数为500
    userInput.setAttribute('maxlength', '500');

    // 字数统计提示（左下角）
    let charCountTip = document.createElement('div');
    charCountTip.style.position = 'absolute';
    charCountTip.style.left = '16px';
    charCountTip.style.bottom = '12px';
    charCountTip.style.fontSize = '12px';
    charCountTip.style.color = '#888';
    charCountTip.style.zIndex = '2';
    userInput.parentNode.appendChild(charCountTip);

    function updateCharCount() {
        const len = userInput.value.length;
        charCountTip.textContent = `${len}/500`;
        if (len >= 500) {
            charCountTip.style.color = '#e74c3c';
        } else {
            charCountTip.style.color = '#888';
        }
    }
    
    userInput.addEventListener('input', updateCharCount);
    updateCharCount();

    // 自动聚焦到输入框
    userInput.focus();
    
    // 输出版本信息到控制台
    console.log('Chat Zenith 前端已加载');
    console.log('API端点:', API_URL);
});